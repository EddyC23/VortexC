#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>
//void consumer0(void* const bufRptr, int streamSizePower) {
//	ULONG_PTR i = 0;
//
//	long long sum = 0;
//	while (i < 1ULL << streamSizePower >> 2) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//	}
//}


//doesnt work/ crashes?


//void producer1(void* const bufWptr, int streamSizePower) {
//	
//	memset(bufWptr, 55, (1ULL << streamSizePower));
//	
//	Vortex::producer_done();
//	std::cout << "Done";
//}

void producer2(void* const bufWptr, int streamSizePower) {
	
	ULONG_PTR i =0;
	__m256i X = _mm256_set1_epi32(1);
	//std::cout << (1ULL << streamSizePower >> 5 << 21);
	while (i < 1ULL << streamSizePower  >> 5) {
		//std::cout << i << "\n";
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i  << 5)), X);
		i++;
	}
	Vortex::producer_done();
}
void producer3(void* const bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower   >> 5) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i  << 5)), X);
		i++;
	}
	Vortex::producer_done();
}


void consumer1(void* const bufRptr, int streamSizePower) {
	ULONG_PTR i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 5) {
		
		auto x = _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i << 5)));
		//std::cout << "The first int at: " << i << " is : " << x.m256i_i32[0]  << "\n"<< x.m256i_i32[1] << "\n" << x.m256i_i32[2] << "\n" << x.m256i_i32[7] << "\n" << x.m256i_i32[6] << "\n";
		sum = _mm256_add_epi32(sum, x);
		i++;
	}
	//std::cout << "done";

} 

int main() {
	bool test = false;
	if (test) {
		void* ptr = _aligned_malloc(1ULL << 30, 64);
		memset(ptr, 0, 1ULL << 30);
		
		while (true) {
			ULONG_PTR i = 0;
			__m256i X = _mm256_set1_epi32(1);
			auto startTime = std::chrono::steady_clock::now();
			while (i < 1ULL << 30 >> 5 >> 21) {
				_mm256_store_si256((__m256i*)((ULONG_PTR)ptr + (i << 5 << 21)), X);
				i++;
			}
			auto endTime = std::chrono::steady_clock::now();
			std::cout <<"STORE " << 1.0 / ((double)((endTime - startTime).count()) / std::pow(10, 9)) << "\n";
			
			i = 0;
			X = _mm256_set1_epi32(1);
			startTime = std::chrono::steady_clock::now();
			while (i < 1ULL << 30 >> 5) {
				_mm256_stream_si256((__m256i*)((ULONG_PTR)ptr + (i << 5)), X);
				i++;
			}
			endTime = std::chrono::steady_clock::now();
			std::cout << "STREAM " << 1.0 / ((double)((endTime - startTime).count()) / std::pow(10, 9)) << "\n";
		}
	
	}
	else {
		ULONGLONG STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER;
		unsigned int L, M, N;

		STREAM_SIZE_POWER = 30, BLOCK_SIZE_PAGE_POWER = 9;
		L = 1, M = 0, N = 2;


		
		Vortex v6(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer2, &consumer1);
		Vortex v7(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer3, &consumer1);


		std::vector<Vortex* > vortexes = { &v6, &v7 };

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

			
			"_mm256_storeu_si256 loop producer, _mm256_add_epi32 loop sum consumer",
			"_mm256_stream_si256 loop producer, _mm256_add_epi32 loop sum consumer"



		};

		size_t numTests = 5;
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
			benchmark << "Average Time: " << (double)(sumTime / numTests) / std::pow(10, 9) << "s\n";
			benchmark << "Average Speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sumTime / numTests) / std::pow(10, 9))) << "GB/s\n\n\n";


			std::cout << descriptions.at(i) << "\n\n";
			std::cout << "Average Time: " << (double)(sumTime / numTests) / std::pow(10, 9) << "s\n";
			std::cout << "Average Speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sumTime / numTests) / std::pow(10, 9))) << "GB/s\n\n\n";



		}

		benchmark << "Total Average Speed: " << vortexes.at(0)->getStreamSizeGB() / ((double)(totalTime / totalNumTests) / std::pow(10, 9)) << "GB/s\n";
		std::cout << "Total Average Speed: " << vortexes.at(0)->getStreamSizeGB() / ((double)(totalTime / totalNumTests) / std::pow(10, 9)) << "GB/s\n";



		times.close();
		benchmark.close();



	}


}