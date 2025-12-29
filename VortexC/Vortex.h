#pragma once
#include <windows.h>
#include <string>
#include <unordered_map>

class Vortex {
public:
	BOOL EnableLockPriveleges(); // ai generated
	std::string GetLastErrorAsString(); // ai generated
	Vortex(uint64_t streamSizePower, uint64_t blockSizePower, unsigned int L, unsigned int M, unsigned int N);
	~Vortex();
	void* getWBuf();
	void* getRBuf();
	static void producer_done();
private:
	static Vortex* instance;
	static LONG WINAPI handler(PEXCEPTION_POINTERS info);
	LONG handle_exception(PEXCEPTION_POINTERS info);

	uint64_t streamSizePower;
	uint64_t streamSizeBytes;
	uint64_t blockSizePower;
	uint64_t blockSizePages;
	uint64_t blockSizeBytes;

	unsigned int L;
	unsigned int M;
	unsigned int N;
	
	void* bufWptr;
	void* bufRptr;

	HANDLE fullSemaphore;
	HANDLE emptySemaphore;
	void acquireSemaphore(HANDLE semaphore);
	void releaseSemaphore(HANDLE semaphore);

	PULONG_PTR arrayPFN;
	std::unordered_map <ULONG_PTR, PULONG_PTR> offsetToPFN;

	uint64_t lastConsUnmap = 0;
	void unmapBlock(void* ptr);
	void mapBlock(void* ptr, PULONG_PTR pageArray);
};