#include "Vortex.h"
#include <Windows.h>
#include <iostream>
#include <thread>
#include <semaphore>
#include <map>
#include <string>
#include <chrono>
Vortex * Vortex::instance = nullptr;


LONG WINAPI Vortex::handler(PEXCEPTION_POINTERS info) {
	return instance->handle_exception(info);
	
}

LONG Vortex::handle_exception(PEXCEPTION_POINTERS info) {
	


	bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
	ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
	bool writeTest = false;
	if (isWriteFault) {
		if (writeTest) {
			ULONG_PTR offset = fptr - (ULONG_PTR)bufWptr;
			ULONGLONG offsetBlock = offset >> (BLOCK_SIZE_PAGE_POWER + PAGE_POWER);
			
			//std::cout << "WBlock: " << offsetBlock << "\n";
			if (offsetBlock >= M + N + L + 1) {
				
				

				offsetToPFN[offsetBlock] = offsetToPFN[lastConsUnmap];
				offsetToPFN.erase(lastConsUnmap);

				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + lastConsUnmap * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, NULL);
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);

				lastConsUnmap++;
			}
			else if (offsetBlock >= L + 1) {
				
				
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
			else {
				
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
		}
		else {
			ULONG_PTR offset = fptr - (ULONG_PTR)bufWptr;
			ULONGLONG offsetBlock = offset >> (BLOCK_SIZE_PAGE_POWER + PAGE_POWER);

			//std::cout << "WBlock: " << offsetBlock << "\n";

			if (offsetBlock >= M + N + L + 1) {
				full.release();
				empty.acquire();

				offsetToPFN[offsetBlock] = offsetToPFN[lastConsUnmap];
				offsetToPFN.erase(lastConsUnmap);

				MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + lastConsUnmap * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, NULL);
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);

				lastConsUnmap++;
			}
			else if (offsetBlock >= L + 1) {
				full.release();
				empty.acquire();
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
			else {
				empty.acquire();
				MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
		}
	}
	else {
		if(writeTest){
			ULONG_PTR offset = fptr - (ULONG_PTR)bufRptr;
			ULONGLONG offsetBlock = offset >> (BLOCK_SIZE_PAGE_POWER + PAGE_POWER);
			//std::cout << "RBlock: " << offsetBlock << "\n";
			
			if (offsetBlock >= L + M + N + 1) {
				offsetToPFN[offsetBlock] = offsetToPFN[offsetBlock - (L + M + N + 1)];
				offsetToPFN.erase(offsetBlock - (L + M + N + 1));
				MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + (offsetBlock - (L + M + N + 1)) * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, NULL);
				
				MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
			else {
				
				MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
			}
		}
		else {
			ULONG_PTR offset = fptr - (ULONG_PTR)bufRptr;
			ULONGLONG offsetBlock = offset >> (BLOCK_SIZE_PAGE_POWER + PAGE_POWER);
			//std::cout << "RBlock: " << offsetBlock << "\n";
			if (offsetBlock >= M + 1) {
				empty.release();
			}
			full.acquire();
			MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, NULL);
			MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + offsetBlock * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES, offsetToPFN[offsetBlock]);
		}
	}

	return EXCEPTION_CONTINUE_EXECUTION;
}
Vortex::Vortex(ULONGLONG STREAM_SIZE_POWER, ULONGLONG BLOCK_SIZE_PAGE_POWER, unsigned int L, unsigned int M, unsigned  int N, void (*producer)(void* const, int), void (*consumer)(void* const, int)) :
	STREAM_SIZE_POWER{ STREAM_SIZE_POWER }, STREAM_SIZE_BYTES{ 1ULL << STREAM_SIZE_POWER }, STREAM_SIZE_GB{double(STREAM_SIZE_BYTES) / std::pow(2, 30)},
	PAGE_POWER{12}, BLOCK_SIZE_PAGE_POWER{ BLOCK_SIZE_PAGE_POWER }, BLOCK_NUM_PAGES{ 1ULL << BLOCK_SIZE_PAGE_POWER }, BLOCK_SIZE_BYTES{ BLOCK_NUM_PAGES << PAGE_POWER }, PFNarray{ new ULONG_PTR[BLOCK_NUM_PAGES * (N + L + M + 1)]},
	L{ L }, M{ M }, N{ N },
	producer{ producer }, consumer{ consumer },
	full{ 0 }, empty{  N + L  },
	bufWptr{ VirtualAlloc(NULL, STREAM_SIZE_BYTES, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE) }, bufRptr{ VirtualAlloc(NULL, STREAM_SIZE_BYTES, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE) }


{
	
	EnableLockPriveleges();
	AddVectoredExceptionHandler(1, handler);

	
	instance = this;
	ULONGLONG numPages = BLOCK_NUM_PAGES * (N + L + M + 1);
	AllocateUserPhysicalPages(GetCurrentProcess(), &numPages, PFNarray);
	for (unsigned int i = 0; i < N + L + M  +  1;i++) {
		
		offsetToPFN[i] = &PFNarray[i * BLOCK_NUM_PAGES];
		
	}
	
}
Vortex::~Vortex() {
	delete[] PFNarray;
}

void Vortex::start() {
	instance = this;
	std::thread produce(producer, bufWptr, STREAM_SIZE_POWER);
	std::thread consume(consumer, bufRptr, STREAM_SIZE_POWER);
	produce.join();
	consume.join();
	//std::cout << "Done.";
}

void Vortex::reset() {
	
	lastConsUnmap = 0;
	
	MapUserPhysicalPages((void*)((ULONGLONG)bufWptr + STREAM_SIZE_BYTES - (N + L + M + 1) * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES * (N + L + M + 1), NULL);
	//why did i need to add this unmap for purely testing producer shouldmt a mew ruim kist override it
	MapUserPhysicalPages((void*)((ULONGLONG)bufRptr + STREAM_SIZE_BYTES - (N + L + M + 1) * BLOCK_SIZE_BYTES), BLOCK_NUM_PAGES * (N + L + M + 1), NULL);

	while(full.try_acquire()){}
	while(empty.try_acquire()){}
	for (unsigned int i = 0; i < N + L; i++) {

		empty.release();

	}
	offsetToPFN.clear();
	for (unsigned int i = 0; i < N + L + M + 1; i++) {

		offsetToPFN[i] = &PFNarray[i * BLOCK_NUM_PAGES];

	}
}

void Vortex::producer_done() {
	//std::cout << "asd";
	for (unsigned int i = 0; i <instance->N + instance->L + 5; i++) {
		instance->full.release();
	}
		
}
double Vortex::getStreamSizeGB() {
	return STREAM_SIZE_GB;
}



BOOL Vortex::EnableLockPriveleges() {
	//sets enable lock privileges
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
std::string Vortex :: GetLastErrorAsString()
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
