#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <chrono>
#include <fstream>

typedef struct StreamData {
	void* ptr;
	uint64_t streamSizePower;
} StreamData, * PointerStreamData;

DWORD WINAPI produceFirst32Bytes(LPVOID parameters) {
	void* bufWptr = ((StreamData*)parameters)->ptr;
	uint64_t streamSizePower = ((StreamData*)parameters)->streamSizePower;
	uint64_t lenBlocks = 1ULL << streamSizePower >> 21;
	uint64_t AVXperBlock = 1ULL << 21 >> 5;
	__m256i x = _mm256_set1_epi32(1);
	
	uint64_t steps = 0;
	uint64_t interval = 1 << 10;
	uint64_t bytesPerGigabyte = pow(10, 9);
	clock_t lastClock = clock();
	clock_t startClock = lastClock;
	for (uint64_t i = 0; i < lenBlocks; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i * AVXperBlock, x);

		if (++steps > interval) {
			clock_t curClock = clock();
			double delay = ((double)curClock - lastClock) / CLOCKS_PER_SEC;
			if (curClock - lastClock == 0) {
				delay = 0.001;
			}
			interval /= delay;
			std::cout << "Progress : " << (double)((i + 1) << 21) / bytesPerGigabyte << " / " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << "GB\n";
			std::cout << "Total Elapsed Time : " << ((double)curClock - startClock) / CLOCKS_PER_SEC << "seconds\n";
			lastClock = curClock;
			steps = 0;
		}
	}
	std::cout << "Progress : " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << " / " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << "GB\n";
	std::cout << "Total Elapsed Time : " << ((double)clock() - startClock) / CLOCKS_PER_SEC << "seconds\n";
	return ERROR_SUCCESS;
}

DWORD WINAPI produceEntireBuffer(LPVOID parameters) {
	void* bufWptr = ((StreamData*)parameters)->ptr;
	uint64_t streamSizePower = ((StreamData*)parameters)->streamSizePower;
	uint64_t lenAVX = 1ULL << streamSizePower >> 5;
	__m256i x = _mm256_set1_epi32(1);

	uint64_t steps = 0;
	uint64_t interval = 1 << 10;
	uint64_t bytesPerGigabyte = pow(10, 9);
	clock_t lastClock = clock();
	clock_t startClock = lastClock;
	
	for (uint64_t i = 0; i < lenAVX; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i, x);

		if (++steps > interval) {
			clock_t curClock = clock();
			double delay = ((double)curClock - lastClock) / CLOCKS_PER_SEC;
			if (curClock - lastClock == 0) {
				delay = 0.001;
			}
			interval /= delay;
			std::cout << "Progress : " << (double)((i + 1) << 5) / bytesPerGigabyte << " / " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << "GB\n";
			std::cout << "Total Elapsed Time : " << ((double)curClock - startClock) / CLOCKS_PER_SEC << "seconds\n";
			lastClock = curClock;
			steps = 0;
		}
	}
	std::cout << "Progress : " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << " / " << (double)(1ULL << streamSizePower) / bytesPerGigabyte << "GB\n";
	std::cout << "Total Elapsed Time : " << ((double)clock() - startClock) / CLOCKS_PER_SEC << "seconds\n";
	return ERROR_SUCCESS;
}

DWORD WINAPI consumeFirst32Bytes(LPVOID parameters) {
	void* bufRptr = ((StreamData*)parameters)->ptr;
	uint64_t streamSizePower = ((StreamData*)parameters)->streamSizePower;
	uint64_t lenBlocks = 1ULL << streamSizePower >> 21;
	uint64_t AVXperBlock = 1ULL << 21 >> 5;
	__m256i sum = _mm256_set1_epi32(0);
	
	for (uint64_t i = 0; i < lenBlocks; i++ ) {
		__m256i* p = (__m256i*)bufRptr;
		__m256i x = _mm256_load_si256(p + i * AVXperBlock);
		sum = _mm256_add_epi32(sum, x);
	}
	if (sum.m256i_i32[0] < 2) {
		std::cout << "This is here to prevent the optimizations for release mode from optimizing away this entire function.\n";
	}
	return ERROR_SUCCESS;
}

DWORD WINAPI consumeEntireBuffer(LPVOID parameters) {
	void* bufRptr = ((StreamData*)parameters)->ptr;
	uint64_t streamSizePower = ((StreamData*)parameters)->streamSizePower;
	uint64_t lenAVX = 1ULL << streamSizePower >> 5;
	__m256i sum = _mm256_set1_epi32(0);
	
	for (uint64_t i = 0; i < lenAVX; i++) {
		__m256i* p = (__m256i*)bufRptr;
		__m256i x = _mm256_load_si256(p + i);
		sum = _mm256_add_epi32(sum, x);
	}
	if (sum.m256i_i32[0] < 2) {
		std::cout << "This is here to prevent the optimizations for release mode from optimizing away this entire function.\n";
	}
	return ERROR_SUCCESS;
}

HANDLE createThread(LPTHREAD_START_ROUTINE function,StreamData& parameters) {
	HANDLE thread = CreateThread(NULL, 0, function, &parameters, 0, NULL);
	if (thread == NULL) {
		std::cout << "create thread failed";
		std::cout << GetLastError();
		exit(-1);
	}
	return thread;
}
void waitForThread(HANDLE thread) {
	if (WaitForSingleObject(thread, INFINITE) == WAIT_FAILED) {
		std::cout << "wait for thread failed";
		std::cout << GetLastError();
		exit(-1);
	}
}

int main() {
	std::ios::sync_with_stdio(false);
	uint64_t streamSizePower, blockSizePower;
	unsigned int L, M, N;

	streamSizePower = 37;
	blockSizePower = 21;
	L = 1, M = 0, N = 2;
	
	Vortex v(streamSizePower, blockSizePower, L, M, N);
	void* bufW = v.getWBuf();
	void* bufR = v.getRBuf();
	StreamData produceParam = { bufW, streamSizePower };
	StreamData consumeParam = { bufR, streamSizePower };
	
	double streamSizeDecimalGB = (1ULL << streamSizePower) / pow(10, 9);
	unsigned int numTests = 3;
	for (unsigned int i = 1; i < numTests + 1; i++) {
		std::cout << "test " << i << " beginning...\n";
		clock_t begin = clock();
		HANDLE produce = createThread(produceEntireBuffer, produceParam);
		HANDLE consume = createThread(consumeEntireBuffer, consumeParam);
		/*HANDLE produce = createThread(produceFirst32Bytes, produceParam);
		HANDLE consume = createThread(consumeFirst32Bytes, consumeParam);*/
		waitForThread(produce);
		v.producer_done();
		waitForThread(consume);
		clock_t end = clock();
		v.reset();

		double speed = streamSizeDecimalGB / ((double)(end - begin) / CLOCKS_PER_SEC);
		std::cout << speed << "GB/s\n\n";
	}
}