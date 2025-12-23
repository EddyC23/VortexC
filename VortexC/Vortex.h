#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <semaphore>

class Vortex {
public:
	
	
	struct Stream {
		void (*consumer)(const void*);
	};
	Vortex(ULONGLONG STREAM_SIZE_POWER,ULONGLONG BLOCK_SIZE_PAGE_POWER, unsigned int L, unsigned int M, unsigned int N, void (*producer)(const void*), void (*consumer)(const void*));
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

	

	const void* bufWptr;
	const void* bufRptr;
	
	std::counting_semaphore<> full;
	std::counting_semaphore<> empty;
	void (*producer)(const void*);
	void (*consumer)(const void*);
	
	std::map <ULONG_PTR, PULONG_PTR> offsetToPFN;

	ULONGLONG lastConsUnmap = 0;

	
	BOOL EnableLockPriveleges(); // AI GENERATED
	std::string GetLastErrorAsString(); // AI GENERATED



};