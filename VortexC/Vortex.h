#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <semaphore>

class Vortex {
public:
	
	
	
	Vortex(ULONGLONG STREAM_SIZE_POWER,ULONGLONG BLOCK_SIZE_PAGE_POWER, unsigned int L, unsigned int M, unsigned int N, void (*producer)(void* const,int ), void (*consumer)(void* const, int));
	~Vortex();
	double getStreamSizeGB();
	void start();
	void reset();
	static void producer_done();
	
private:

	static LONG WINAPI handler(PEXCEPTION_POINTERS info);
	static Vortex* instance; 

	LONG handle_exception(PEXCEPTION_POINTERS info);
	
	const unsigned int L;
	const unsigned int M;
	const unsigned int N;

	const ULONGLONG PAGE_POWER;
	const ULONGLONG STREAM_SIZE_POWER;
	const ULONGLONG STREAM_SIZE_BYTES;
	const double STREAM_SIZE_GB;
	const ULONGLONG BLOCK_SIZE_PAGE_POWER;
	const ULONGLONG BLOCK_NUM_PAGES;
	const ULONGLONG BLOCK_SIZE_BYTES;
	const PULONG_PTR PFNarray;

	

	void* const bufWptr;
	void* const bufRptr;
	
	std::counting_semaphore<> full;
	std::counting_semaphore<> empty;
	void (*producer)(void* const, int);
	void (*consumer)(void* const, int);
	
	std::map <ULONG_PTR, PULONG_PTR> offsetToPFN;

	ULONGLONG lastConsUnmap = 0;

	
	BOOL EnableLockPriveleges(); // AI GENERATED
	std::string GetLastErrorAsString(); // AI GENERATED

	bool lastBlock;

};