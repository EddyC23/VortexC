#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>



void produceFirst32Bytes(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t lenAVX = 1ULL << streamSizePower >> 5;
	uint64_t AVXperBlock = 1ULL << 21 >> 5;
	__m256i x = _mm256_set1_epi32(1);

	for (uint64_t i = 0; i < lenAVX; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i * AVXperBlock, x);
	}
	
}

void produceEntireBuffer(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t lenAVX = 1ULL << streamSizePower >> 5;
	__m256i x = _mm256_set1_epi32(1);

	for (uint64_t i = 0; i < lenAVX; i++) {
		__m256i* p = (__m256i*)bufWptr;
		_mm256_store_si256(p + i, x);
		
	}
	
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
	if (sum.m256i_i32[0] > 2) {
		std::cout << "This is here to prevent the optimizations for release mode from optimizing away this entire function.";
	}

}

void consumeEntireBuffer(void* const bufRptr, uint64_t streamSizePower) {
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
}



int main() {
	
	uint64_t streamSizePower, blockSizePower;
	unsigned int L, M, N;

	streamSizePower = 30;
	blockSizePower = 21;
	L = 1, M = 0, N = 2;
	
	Vortex v(streamSizePower, blockSizePower, L, M, N);
	std::cout << v.GetLastErrorAsString();
	void* bufW = v.getWBuf();
	void* bufR = v.getRBuf();

	auto start = std::chrono::steady_clock::now();
	std::thread produce(produceEntireBuffer, bufW, streamSizePower);
	std::thread consume(consumeEntireBuffer, bufR, streamSizePower);
	produce.join();
	Vortex::producer_done();
	consume.join();
	auto end = std::chrono::steady_clock::now();
	std::cout << 1.0/((end - start).count() / pow(10, 9)) << "GB/s";

	



}