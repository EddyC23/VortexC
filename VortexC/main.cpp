#include <iostream>
#include <windows.h>
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>


//Sets lock pages privileges
BOOL EnableLockPagesPrivilege() {
	HANDLE hToken;
	LUID luid;
	TOKEN_PRIVILEGES tp;

	// 1. Open the process token
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
		printf("OpenProcessToken failed. Error: %lu\n", GetLastError());
		return FALSE;
	}

	// 2. Get the LUID for "SeLockMemoryPrivilege"
	if (!LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &luid)) {
		printf("LookupPrivilegeValue failed. Error: %lu\n", GetLastError());
		CloseHandle(hToken);
		return FALSE;
	}

	// 3. Enable the privilege
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
		printf("AdjustTokenPrivileges failed. Error: %lu\n", GetLastError());
		CloseHandle(hToken);
		return FALSE;
	}

	// 4. Check if it actually worked (AdjustTokenPrivileges returns TRUE even if it failed to add the right)
	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
		printf("The token does not have the specified privilege. \n");
		printf("PLEASE NOTE: You must grant 'Lock pages in memory' in Local Security Policy (secpol.msc) first!\n");
		CloseHandle(hToken);
		return FALSE;
	}

	CloseHandle(hToken);
	return TRUE;
}
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}


const int L = 0;
const int N = 1;
const int M = -1; 
const ULONG_PTR NUM_PAGES = 2;
const ULONG_PTR B = NUM_PAGES * 4096; //Block size : num of bytes per block 
const void* bufW = VirtualAlloc(NULL, 1ULL << 40, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
const void *  bufR = VirtualAlloc(NULL,1ULL << 40, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
std::map<ULONG_PTR, PULONG_PTR> offsetToPFN;


std::counting_semaphore semFull(0);
std::counting_semaphore semEmpty(N+L);


LONG WINAPI handler(PEXCEPTION_POINTERS info) {
	std::cout << "Fault Handler Called\n";
	
	ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
	ULONG_PTR bufWptr = (ULONG_PTR)bufW;
	ULONG_PTR bufRptr = (ULONG_PTR)bufR;
	bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
	
	/*00010000
	11110000
	01111111*/
	if (isWriteFault) {
		//8b and with 8byte 
		std::cout << "Write Fault\n";
		std::cout << fptr - bufWptr;
		ULONG_PTR offset = (fptr - bufWptr)&(~(B-1)); 
		if (offset == 0) {
			semEmpty.acquire();
			for (size_t i = 0; i < M + N + L + 1; i++) {

				PULONG_PTR PFNarr = new ULONG_PTR[NUM_PAGES];
				ULONG_PTR numberOfPages = NUM_PAGES;
				AllocateUserPhysicalPages(GetCurrentProcess(), &numberOfPages, PFNarr);

				LPVOID ptr = (LPVOID)bufW;
				MapUserPhysicalPages(ptr, NUM_PAGES, PFNarr);
				offsetToPFN[i * B] = PFNarr;
			}
			return EXCEPTION_CONTINUE_EXECUTION;
		}else if (offset > 0) {
				MapUserPhysicalPages((void *)(bufWptr + offset - N * B), NUM_PAGES, NULL);
			
				semFull.release();
		}
		
		
		std::cout << "Waiting For Empty Block\n";
		semEmpty.acquire();
		std::cout << "Empty Block Acquired\n";
		offsetToPFN[offset] = offsetToPFN[offset - B];
		MapUserPhysicalPages((void*)(bufWptr + offset), NUM_PAGES, offsetToPFN[offset - N * B]);
		
		offsetToPFN.erase(offset - B);
		
		
	}
	else {
		ULONG_PTR offset = (fptr - bufRptr) & (~(B - 1));
		if (offset > 0) {
			MapUserPhysicalPages((void*)(bufRptr + offset - (1 * B)), NUM_PAGES, NULL);
			semEmpty.release();
		}
		std::cout << "Waiting For Full Block\n";
		semFull.acquire();
		MapUserPhysicalPages((void*)(bufRptr + offset), NUM_PAGES, offsetToPFN[offset]);
		
		
		std::cout << "Full Block Acquired\n";

	}
	return EXCEPTION_CONTINUE_EXECUTION;
}


void producer() {
	
	size_t i = 0;
	
	while (true) {
		((int*)bufW)[i] = i;
		i++;
		std::cout<<"Current is: " << i << "\n";
	}
}

void consumer() {
	size_t i = 0;
	
	long long sum = 0;
	while (true) {
		sum += ((int*)bufR)[i];
		i++;
		std::cout << "Sum is: " << sum<<"\n";
	}
	
}



int main() {
	
	
	EnableLockPagesPrivilege();
	AddVectoredExceptionHandler(1, handler);
	
	
	
	std::thread produce(producer);
	std::thread consume(consumer);
	produce.join();
	consume.join();
	std::cout << "Done ! ";
	
	
		



	




}


