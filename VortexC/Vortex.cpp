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
	
	char* bufW = (char*)bufWptr;
	char* bufR = (char*)bufRptr;

	bool isWriteFault = info->ExceptionRecord->ExceptionInformation[0];
	ULONG_PTR fptr = info->ExceptionRecord->ExceptionInformation[1];
	
	if (isWriteFault) {
			

			ULONG_PTR offset = fptr - (ULONG_PTR)bufWptr;
			ULONGLONG offsetBlock = offset >> blockSizePower;
			//std::cout << "wf" << offsetBlock;
			if (offsetBlock == 0) {
				for (unsigned int i = 0; i < N + L + M + 1; i++) {
					offsetToPFN[i] = arrayPFN + i * blockSizePages;
				}
			}

			if (offsetBlock >= M + N + L + 1) {
				releaseSemaphore(fullSemaphore);
				acquireSemaphore(emptySemaphore);
				offsetToPFN[offsetBlock] = offsetToPFN[lastConsUnmap];
				offsetToPFN.erase(lastConsUnmap);
				unmapBlock(bufR + lastConsUnmap * blockSizeBytes);
				mapBlock(bufW + offsetBlock * blockSizeBytes, offsetToPFN[offsetBlock]);
				lastConsUnmap++;
			}
			else if (offsetBlock >= L + 1) {
				releaseSemaphore(fullSemaphore);
				acquireSemaphore(emptySemaphore);
				mapBlock(bufW + offsetBlock * blockSizeBytes, offsetToPFN[offsetBlock]);
			}
			else {
				acquireSemaphore(emptySemaphore);
				mapBlock(bufW + offsetBlock * blockSizeBytes, offsetToPFN[offsetBlock]);
			}

	}
	else {
		
		
			ULONG_PTR offset = fptr - (ULONG_PTR)bufRptr;
			ULONGLONG offsetBlock = offset >> blockSizePower;
			//std::cout << "rf" << offsetBlock;
			if (offsetBlock >= M + 1) {
				releaseSemaphore(emptySemaphore);
			}
			acquireSemaphore(fullSemaphore);
			unmapBlock(bufW + offsetBlock * blockSizeBytes);
			mapBlock(bufR + offsetBlock * blockSizeBytes, offsetToPFN[offsetBlock]);
		
	}
	
	return EXCEPTION_CONTINUE_EXECUTION;
}
Vortex::Vortex(uint64_t streamSizePower, uint64_t blockSizePower, unsigned int L, unsigned int M, unsigned  int N)
{

	EnableLockPriveleges();
	AddVectoredExceptionHandler(1, handler);
	instance = this;

	this->streamSizePower = streamSizePower;
	this->streamSizeBytes = 1ULL << streamSizePower;
	this->blockSizePower = blockSizePower;
	this->blockSizePages = 1ULL << (blockSizePower - 12);
	this->blockSizeBytes = 1ULL << blockSizePower;
	this->L = L;
	this->M = M;
	this->N = N;
	this->fullSemaphore = CreateSemaphore(NULL, 0, N + L, NULL);
	this->emptySemaphore = CreateSemaphore(NULL, N+L, N + L, NULL);
	this->bufWptr = VirtualAlloc(NULL, streamSizeBytes, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
	this->bufRptr = VirtualAlloc(NULL, streamSizeBytes, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
	
	if (blockSizePower < 12) {
		std::cout << "block size power has to be at least 12";
		exit(-1);
	}

	ULONGLONG numPages = (blockSizePages) * (N + L + M + 1);
	arrayPFN = new ULONG_PTR[numPages];
	if (!AllocateUserPhysicalPages(GetCurrentProcess(), &numPages, arrayPFN)) {
		std::cout << "allocate user physical pages failed\n";
		std::cout << GetLastError();
		exit(-1);
	}
	if (numPages != (blockSizePages) * (N + L + M + 1)) {
		std::cout << "allocate user physical pages allocated incorrect number of pages";
		exit(-1);
	}
	
	if (bufWptr == NULL || bufRptr == NULL) {
		std::cout << "virtual alloc failed";
		std::cout << GetLastError();
		exit(-1);
	}
	if (fullSemaphore == NULL || emptySemaphore == NULL) {
		std::cout << "creating semaphores failed";
		std::cout << GetLastError();
		exit(-1);
	}
	
	
}
Vortex::~Vortex() {
	delete[] arrayPFN;
}

void* Vortex::getWBuf()
{
	return bufWptr;
}
void* Vortex::getRBuf()
{
	return bufRptr;
}





void Vortex::producer_done() {
	
	for (unsigned int i = 0; i <instance->N + instance->L; i++) {
		instance->releaseSemaphore(instance->fullSemaphore);
	}
		
}


void Vortex::acquireSemaphore(HANDLE semaphore) {
	if (WaitForSingleObject(semaphore, INFINITE) == WAIT_FAILED) {
		std::cout << "aqcuire semaphore failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
void Vortex::releaseSemaphore(HANDLE semaphore) {
	LPLONG lpPreviousCount = 0;
	if (!ReleaseSemaphore(semaphore, 1, lpPreviousCount)) {
		std::cout << "release semaphore failed";
		std::cout << GetLastError();
		exit(-1);
	}
}

void Vortex::unmapBlock(void* ptr)
{
	if (!MapUserPhysicalPages(ptr, blockSizePages, NULL)) {
		std::cout << "unmap block failed";
		std::cout << GetLastError();
		exit(-1);
	}
}
void Vortex::mapBlock(void* ptr, PULONG_PTR pageArray)
{
	if (!MapUserPhysicalPages(ptr, blockSizePages, pageArray)) {
		std::cout << "map block failed";
		std::cout << GetLastError();
		exit(-1);
	}
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
