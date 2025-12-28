#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>



//PRODUCE ONLY FIRST 32BYTES OF EVERY BLOCK WITH STORE
void producer0(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 21) {
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 21)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ONLY FIRST 32BYTES OF EVERY BLOCK WITH STREAM
void producer1(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 21) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 21)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ENTIRE BUFFER WITH STORE
void producer2(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 5) {
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ENTIRE BUFFER WITH STREAM
void producer3(void* const bufWptr, uint64_t streamSizePower) {

	uint64_t i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 5) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}

//CONSUME FIRST 32BYTES OF EACH BLOCK WITH LOAD
void consumer0(void* const bufRptr, uint64_t streamSizePower) {
	uint64_t i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 21) {

		auto x = _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i << 21)));
		sum = _mm256_add_epi32(sum, x);
		i++;

	}
	if (sum.m256i_i32[0] > 2) {

	}

}

//CONSUME ENTIRE BUFFER WITH LOAD
void consumer1(void* const bufRptr, uint64_t streamSizePower) {
	uint64_t i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 5) {

		auto x = _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i << 5)));
		sum = _mm256_add_epi32(sum, x);
		i++;

	}

	if (sum.m256i_i32[0] > 2) {

	}
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

	auto start = std::chrono::steady_clock::now();
	std::thread produce(&producer2, bufW, streamSizePower);
	std::thread consume(&consumer1, bufR, streamSizePower);
	produce.join();
	consume.join();
	auto end = std::chrono::steady_clock::now();
	std::cout << 1.0/((end - start).count() / pow(10, 9)) << "GB/s";

	



}