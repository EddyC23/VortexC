#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <chrono>
#include <fstream>


//around 7-8GB/s as of right now//debug mode still around 3GB/s

typedef struct StreamData {
	void* ptr;
	uint64_t streamSizePower;
} StreamData, * PointerStreamData;

void produceFirst32Bytes(void* const bufWptr, uint64_t streamSizePower) {
	uint64_t lenBlocks = 1ULL << streamSizePower >> 21;
	uint64_t AVXperBlock = 1ULL << 21 >> 5;
	__m256i x = _mm256_set1_epi32(1);

	for (uint64_t i = 0; i < lenBlocks; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i * AVXperBlock, x);
	}
}

DWORD WINAPI produceEntireBuffer(LPVOID parameters) {
	void* bufWptr = ((StreamData*)parameters)->ptr;
	uint64_t streamSizePower = ((StreamData*)parameters)->streamSizePower;
	uint64_t lenAVX = 1ULL << streamSizePower >> 5;
	__m256i x = _mm256_set1_epi32(1);

	for (uint64_t i = 0; i < lenAVX; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i, x);
	}
	return ERROR_SUCCESS;
}

void consumeFirst32Bytes(void* const bufRptr, uint64_t streamSizePower) {
	uint64_t lenBlocks = 1ULL << streamSizePower >> 21;
	uint64_t AVXperBlock = 1ULL << 21 >> 5;
	__m256i sum = _mm256_set1_epi32(0);
	
	for (uint64_t i = 0; i < lenBlocks; i++ ) {
		__m256i* p = (__m256i*)bufRptr;
		__m256i x = _mm256_load_si256(p + i * AVXperBlock);
		sum = _mm256_add_epi32(sum, x);
	}
	if (sum.m256i_i32[0] < 2) {
		std::cout << "This is here to prevent the optimizations for release mode from optimizing away this entire function.";
	}
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
		std::cout << "This is here to prevent the optimizations for release mode from optimizing away this entire function.";
	}
	return ERROR_SUCCESS;
}



int main() {
	
	uint64_t streamSizePower, blockSizePower;
	unsigned int L, M, N;

	streamSizePower = 30;
	blockSizePower = 21;
	L = 1, M = 0, N = 2;
	
	Vortex v(streamSizePower, blockSizePower, L, M, N);
	void* bufW = v.getWBuf();
	void* bufR = v.getRBuf();
	StreamData produceParam = { bufW, streamSizePower };
	StreamData consumeParam = { bufR, streamSizePower };
	
	clock_t begin = clock();
	HANDLE produce = CreateThread(NULL, 0, produceEntireBuffer, &produceParam, 0, NULL);
	HANDLE consume = CreateThread(NULL, 0, consumeEntireBuffer, &consumeParam, 0, NULL);
	WaitForSingleObject(produce, INFINITE);
	Vortex::producer_done();
	WaitForSingleObject(consume, INFINITE);
	clock_t end = clock();

	std::cout << 1/((begin -  end) / CLOCKS_PER_SEC) << "GB/s";

	



}