#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>



//PRODUCE ONLY FIRST 32BYTES OF EVERY BLOCK WITH STORE
void producer0(void* const bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 21) {
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 21)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ONLY FIRST 32BYTES OF EVERY BLOCK WITH STREAM
void producer1(void* const bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower >> 21) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 21)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ENTIRE BUFFER WITH STORE
void producer2(void* const bufWptr, int streamSizePower) {
	
	ULONG_PTR i =0;
	__m256i X = _mm256_set1_epi32(1);
	
	while (i < 1ULL << streamSizePower  >> 5) {
		_mm256_store_si256((__m256i*)((ULONG_PTR)bufWptr + (i  << 5)), X);
		i++;
	}
	Vortex::producer_done();
}

//PRODUCE ENTIRE BUFFER WITH STREAM
void producer3(void* const bufWptr, int streamSizePower) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << streamSizePower   >> 5) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i  << 5)), X);
		i++;
	}
	Vortex::producer_done();
}

//CONSUME FIRST 32BYTES OF EACH BLOCK WITH LOAD
void consumer0(void* const bufRptr, int streamSizePower) {
	ULONG_PTR i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 21) {

		auto x = _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i <<21)));
		sum = _mm256_add_epi32(sum, x);
		i++;
	}


}

//CONSUME ENTIRE BUFFER WITH LOAD
void consumer1(void* const bufRptr, int streamSizePower) {
	ULONG_PTR i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << streamSizePower >> 5) {
		
		auto x = _mm256_load_si256((__m256i*)((ULONG_PTR)bufRptr + (i << 5)));
		sum = _mm256_add_epi32(sum, x);
		i++;
	}
	

} 



int main() {
	 
		ULONGLONG STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER;
		unsigned int L, M, N;

		STREAM_SIZE_POWER = 30, BLOCK_SIZE_PAGE_POWER = 9;
		L = 1, M = 0, N = 2;


		
		Vortex v0(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer1, &consumer1);
		Vortex v1(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer3, &consumer1);
		



		std::vector<Vortex* > vortexes = { &v0, &v1 };

		std::ofstream times("Times.txt", std::ios_base::app);
		std::ofstream benchmark("Benchmark.txt", std::ios_base::app);
		times << "///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n";
		benchmark << "///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////\n";

		/*std::string runDesc;
		std::getline(std::cin, runDesc);
		times << runDesc << "\n";
		benchmark << runDesc << "\n";*/

		std::vector<std::string> descriptions =
		{

			
			"1",
			"2"



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

/*bool test = false;
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
	
	}*/
/*void producer1(void* const bufWptr, int streamSizePower) {
	ULONG_PTR i = 0;
	while (i < 1ULL << streamSizePower >> 21) {
		memset((void*)((ULONG_PTR)bufWptr + (i << 21)), 55, 1ULL << 21);
		i++;
	}Vortex::producer_done();
}
*/