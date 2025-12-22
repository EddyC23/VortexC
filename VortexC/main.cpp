#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>



//Returns the last Win32 error, in string format. Returns an empty string if there is no error.



const int L = 0;
const int N = 2;
const int M = 3; 

const ULONG_PTR NUM_PAGES = 2;
const ULONG_PTR B = NUM_PAGES * 4096; //Block size : num of bytes per block
const ULONG_PTR BLOCK_SIZE_POWER = 13;

const void* bufW = VirtualAlloc(NULL, 1ULL << 40, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
const void *  bufR = VirtualAlloc(NULL,1ULL << 40, MEM_RESERVE | MEM_PHYSICAL, PAGE_READWRITE);
std::map<ULONG_PTR, PULONG_PTR> offsetToPFN;
//create a test case that comes back with odd write with consumer.. and M N L + 1
//can use std:;chrono to time, benchmarks can be smaller so they finish in  areasonable amount of time
//m is consumer comeback region
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
		ULONG_PTR offset = (fptr - bufWptr) & (~(B - 1));
		ULONGLONG offsetBlock = offset >> BLOCK_SIZE_POWER;
		std::cout << "Write Fault at Block: " << offsetBlock << "\n";
		
		if (offset == 0) {
			semEmpty.acquire();
			for (ULONG_PTR i = 0; i < M + N + L + 1; i++) {

				PULONG_PTR PFNarr = new ULONG_PTR[NUM_PAGES];
				ULONG_PTR numberOfPages = NUM_PAGES;
				ULONG_PTR initOffset = i * B;


				AllocateUserPhysicalPages(GetCurrentProcess(), &numberOfPages, PFNarr);


				offsetToPFN[initOffset] = PFNarr;

				MapUserPhysicalPages((LPVOID)(bufWptr + initOffset), NUM_PAGES, PFNarr);

			}
			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else if (offset > 0 && offset < N * B) {
		}
		else if (offset  >= N * B) {
				//unmap first block in write buffer for consumer
				MapUserPhysicalPages((void *)(bufWptr + offset - N * B), NUM_PAGES, NULL);
			
				semFull.release();
		}
		
		
		std::cout << "Waiting For Empty Block\n";
		semEmpty.acquire();
		std::cout << "Empty Block Acquired\n";
		
		//map front of the read buffer to the fault place of the write buffer
		MapUserPhysicalPages((void*)(bufWptr + offset), NUM_PAGES, offsetToPFN[offset - (N + M + 1) * B]);
		offsetToPFN[offset] = offsetToPFN[offset - (N + L) * B];
		offsetToPFN.erase(offset - (N + M + 1) * B);
		
		
	}
	else {
		ULONG_PTR offset = (fptr - bufRptr) & (~(B - 1));
		ULONGLONG offsetBlock = offset >> BLOCK_SIZE_POWER;
		std::cout << "Read Fault at Block: " << offsetBlock << "\n";
		if (offset >= (M + 1)  * B) {
			//unmap the front of the read buffer for the producer to take phys page
			MapUserPhysicalPages((void*)(bufRptr + offset - ((M + 1) * B)), NUM_PAGES, NULL);
			semEmpty.release();
		}
		std::cout << "Waiting For Full Block\n";
		semFull.acquire();
		//map the front of the write buffer to end of the read buffer where fault occured
		MapUserPhysicalPages((void*)(bufRptr + offset), NUM_PAGES, offsetToPFN[offset]);
		
		
		std::cout << "Full Block Acquired\n";

	}
	return EXCEPTION_CONTINUE_EXECUTION;
}


void producer(const void * bufWptr) {
	
	ULONGLONG i = 0;
	
	while (true) {
		((int*)bufWptr)[i] = i;
		i++;
		std::cout<<"Current is: " << i << "\n";
	}
}

void consumer(const void* bufRptr) {
	ULONGLONG i = 0;
	
	long long sum = 0;
	while (true) {
		sum += ((int*)bufRptr)[i];
		i++;
		std::cout << "Sum is: " << sum<<"\n";
	}
	
}
void testproducer(const void* bufWptr) {

	ULONGLONG i = 0;

	while (i < 16384) {
		((int*)bufWptr)[i] = i;
		i++;
		std::cout << "Current is: " << i << "\n";
	}
	((int*)bufWptr)[i] = i;
	std::cout << "Producer Done.";
}

void testconsumer(const void* bufRptr) {
	ULONGLONG i = 0;

	long long sum = 0;
	while (i<16384) {
		sum += ((int*)bufRptr)[i];
		i++;
		std::cout << "Sum is: " << sum << "\n";
	}
	std::cout << "Consumer Done.";
}



int main() {
	
	ULONGLONG STREAM_SIZE_POWER = 40, BLOCK_SIZE_PAGE_POWER = 0;
	unsigned int L = 2, M = 3, N = 4;
	Vortex v(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER , L, M, N, &producer, &consumer);
	STREAM_SIZE_POWER =  16, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v1(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer, &testconsumer);
	v1.start();
}


