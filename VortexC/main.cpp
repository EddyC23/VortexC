#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>
//void consumer0(const void* bufRptr, int streamSizePower) {
//	ULONG_PTR i = 0;
//
//	long long sum = 0;
//	while (i < 1ULL << streamSizePower >> 2) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//	}
//}
void producer1(const void* bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;

	while (i < 1ULL << streamSizePower >> 5) {
		memset((void*)((ULONG_PTR)bufWptr + (i << 5)), 1, 1ULL << 5);
		i++;
	}
	Vortex::producer_done();
}

void producer2(const void* bufWptr, int streamSizePower) {
	
	ULONG_PTR i =0;
	__m256i X = _mm256_set1_epi32(1);
	
	while (i < 1ULL << streamSizePower >> 5) {
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}
void producer3(const void* bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 5) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}


void consumer1(const void* bufRptr, int streamSizePower) {
	ULONG_PTR i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 5) {
		
		
		sum = _mm256_add_epi32(sum, _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i << 5))));
		i++;
	}

} 

int main() {
	
	ULONGLONG STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER;
	unsigned int L, M, N;

	STREAM_SIZE_POWER = 34, BLOCK_SIZE_PAGE_POWER = 9;
	L = 1, M = 0, N = 2;

	
	Vortex v5(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer1, &consumer1);
	Vortex v6(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer2, &consumer1);
	Vortex v7(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer3, &consumer1);

	
	std::vector<Vortex* > vortexes = {&v5,&v6, &v7 };

	std::ofstream times("Times.txt", std::ios_base::app);
	std::ofstream benchmark("Benchmark.txt", std::ios_base::app);
	times << "///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n";
	benchmark << "///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n";
	
	std::string runDesc;
	std::getline(std::cin, runDesc);
	times << runDesc << "\n";
	benchmark << runDesc << "\n";

	std::vector<std::string> descriptions =
	{
		
		"memset 32byte producer, _mm256_add_epi32 loop sum consumer",
		"_mm256_storeu_si256 loop producer, _mm256_add_epi32 loop sum consumer",
		"_mm256_stream_si256 loop producer, _mm256_add_epi32 loop sum consumer"
		
		

	};
	
	size_t numTests = 1;
	ULONGLONG totalTime = 0;
	ULONGLONG totalNumTests = vortexes.size() * numTests;

	for (size_t i = 0; i < vortexes.size(); i++) {

		

		ULONGLONG sumTime = 0;
		
		

		times << descriptions.at(i) << "\n";

		for (size_t j = 0; j < numTests; j++) {

			

			auto startTime = std::chrono::steady_clock::now();
			vortexes.at(i)->start();
			auto endTime = std::chrono::steady_clock::now();
			vortexes.at(i)->reset();
			
			
			
			sumTime += (endTime - startTime).count();
			totalTime += (endTime - startTime).count();

			times << endTime - startTime << "\n";
			

			
		}

		benchmark << descriptions.at(i) << "\n\n";
		benchmark << "Average Time: " <<  (double)(sumTime / numTests) / std::pow(10, 9) << "s\n";
		benchmark << "Average Speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sumTime / numTests) / std::pow(10, 9))) << "GB/s\n\n\n";
		

		std::cout << descriptions.at(i) << "\n\n";
		std::cout << "Average Time: "  << (double)(sumTime / numTests) / std::pow(10, 9) << "s\n";
		std::cout << "Average Speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sumTime / numTests) / std::pow(10, 9))) << "GB/s\n\n\n";

		
		
	}
	
	benchmark << "Total Average Speed: " << vortexes.at(0)->getStreamSizeGB() / ((double)(totalTime/totalNumTests) / std::pow(10, 9)) << "GB/s\n";
	std::cout << "Total Average Speed: " << vortexes.at(0)->getStreamSizeGB() / ((double)(totalTime/ totalNumTests) / std::pow(10, 9)) << "GB/s\n";
	
	
	
	times.close();
	benchmark.close();
	


}


