#include <iostream>
#include <windows.h>
#include "Vortex.h"
#include <thread>
#include <chrono>
#include <semaphore>
#include <map>
#include <fstream>
//producers and consumers need to have explicit length
//void producer(const void * bufWptr) {
//	
//	ULONGLONG i = 0;
//	
//	while (true) {
//		((int*)bufWptr)[i] = i;
//		i++;
//		std::cout<<"Current is: " << i << "\n";
//	}
//}
//
//void consumer(const void* bufRptr) {
//	ULONGLONG i = 0;
//	
//	long long sum = 0;
//	while (true) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//		std::cout << "Sum is: " << sum<<"\n";
//	}
//	
//}
//void testproducer(const void* bufWptr) {
//
//	ULONGLONG i = 0;
//
//	while (i < 16384) {
//		((int*)bufWptr)[i] = i;
//		i++;
//		std::cout << "Current is: " << i << "\n";
//	}
//	/*((int*)bufWptr)[i] = i;*/
//	std::cout << "Producer Done.";
//	Vortex::producer_done();
//}
//
//void testconsumer(const void* bufRptr) {
//	ULONGLONG i = 0;
//
//	long long sum = 0;
//	while (i<16384) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//		std::cout << "Sum is: " << sum << "\n";
//	}
//	std::cout << "Consumer Done.";
//}

//void testproducer0(const void* bufWptr) {
//
//	ULONGLONG i = 0;
//
//	while (i < 1ULL << 20 >> 2) {
//		((int*)bufWptr)[i] = i;
//		
//		i++;
//	}
//	Vortex::producer_done();
//	
//}
//
//void testconsumer0(const void* bufRptr) {
//	ULONGLONG i = 0;
//
//	long long sum = 0;
//	while (i < 1ULL<< 20 >>2) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//	}
//}
//
//void testproducer1(const void* bufWptr) {
//
//	ULONGLONG i = 0;
//
//	while (i < (1ULL << 20) - 3) {
//		
//		
//		memset((void *)((ULONG_PTR)bufWptr + i), 'a', 3);
//		i += 3;
//	}
//	
//	((char*)bufWptr)[i] = 'z';
//	Vortex::producer_done();
//}
//
//void testconsumer1(const void* bufRptr) {
//	ULONGLONG i = 0;
//	
//	while (i < 1ULL<<20) {
//		if (((char*)bufRptr)[i]) {}
//		i++;
//	}
//}
//
//void testproducer2(const void* bufWptr) {
//
//	ULONGLONG i = 0;
//
//	while (i < 1ULL << 27 >> 2) {
//		((int*)bufWptr)[i] = i;
//		i++;
//	}
//	Vortex::producer_done();
//}
//
//void testconsumer2(const void* bufRptr) {
//	ULONGLONG i = 0;
//
//	long long sum = 0;
//	while (i < 1ULL << 27 >> 2) {
//		sum += ((int*)bufRptr)[i];
//		i++;
//	}
//}
//
//void testproducer3(const void* bufWptr) {
//
//	ULONGLONG i = 0;
//
//	while (i < (1ULL << 27) - 3) {
//
//
//		memset((void*)((ULONG_PTR)bufWptr + i), 'a', 3);
//		i += 3;
//	}
//
//	((char*)bufWptr)[i] = 'z';
//	Vortex::producer_done();
//}
//
//void testconsumer3(const void* bufRptr) {
//	ULONGLONG i = 0;
//
//	while (i < 1ULL << 27) {
//		if (((char*)bufRptr)[i]) {}
//		i++;
//	}
//}

void producer0(const void* bufWptr) {
	
	ULONG_PTR i =0;
	__m256i X = _mm256_set1_epi32(1);
	
	while (i < 1ULL << 27 >> 5) {	
		_mm256_storeu_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}
void producer1(const void* bufWptr) {

	ULONG_PTR i = 0;
	__m256i X = _mm256_set1_epi32(1);

	while (i < 1ULL << 27 >> 5) {
		_mm256_stream_si256((__m256i*)((ULONG_PTR)bufWptr + (i << 5)), X);
		i++;
	}
	Vortex::producer_done();
}

void consumer0(const void* bufRptr) {
	ULONG_PTR i = 0;
	__m256i sum = _mm256_set1_epi32(0);

	while (i < 1ULL << 27 >> 5) {
		
		
		sum = _mm256_add_epi32(sum, _mm256_loadu_epi32((void*)((ULONG_PTR)bufRptr + (i << 5))));
		i++;
	}

} 

int main() {
	
	/*ULONGLONG STREAM_SIZE_POWER = 40, BLOCK_SIZE_PAGE_POWER = 0;
	unsigned int L = 2, M = 3, N = 4;
	Vortex v(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER , L, M, N, &producer, &consumer);
	STREAM_SIZE_POWER =  16, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v1(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer, &testconsumer);
	STREAM_SIZE_POWER = 16, BLOCK_SIZE_PAGE_POWER = 0;
	L = 0, M = 0, N = 1;
	Vortex v2(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer, &testconsumer);*/
	//still some bugs with special cases v2 hmmmm and also having multiple decalrations of vortex objects
	//v1.start();

	//benchmarks
	/*STREAM_SIZE_POWER = 20, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v0(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer0, &testconsumer0);

	STREAM_SIZE_POWER = 20, BLOCK_SIZE_PAGE_POWER = 3;
	L = 2, M = 3, N = 4;
	Vortex v1(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer0, &testconsumer0);

	STREAM_SIZE_POWER = 20, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v2(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer1, &testconsumer1);

	STREAM_SIZE_POWER = 20, BLOCK_SIZE_PAGE_POWER = 3;
	L = 2, M = 3, N = 4;
	Vortex v3(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer1, &testconsumer1);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v4(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer2, &testconsumer2);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 3;
	L = 2, M = 3, N = 4;
	Vortex v5(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer2, &testconsumer2);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 0;
	L = 2, M = 3, N = 4;
	Vortex v6(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer3, &testconsumer3);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 3;
	L = 2, M = 3, N = 4;
	Vortex v7(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer3, &testconsumer3);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 8; 
	L = 2, M = 3, N = 4;
	Vortex v8(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer2, &testconsumer2);

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 8;
	L = 2, M = 3, N = 4;
	Vortex v9(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &testproducer3, &testconsumer3);*/

	

	ULONGLONG STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER;
	unsigned int L, M, N;

	STREAM_SIZE_POWER = 27, BLOCK_SIZE_PAGE_POWER = 8;
	L = 2, M = 3, N = 4;

	Vortex v0(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer0, &consumer0);
	Vortex v1(STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER, L, M, N, &producer1, &consumer0);
	std::vector<Vortex* > vortexes = { &v0, &v1 };

	std::ofstream times("Times.txt", std::ios_base::out);
	std::ofstream benchmark("Benchmark.txt", std::ios_base::out);
	std::vector<std::string> descriptions =
	{
		"_mm256_storeu_si256",
		"_mm256_storeu_si256"

	};
	
	
	

	for (size_t i = 0; i < vortexes.size(); i++) {

		

		ULONGLONG sum = 0;
		ULONGLONG first = 0;
		size_t numTests = 10;

		times << descriptions.at(i) << "\n";

		for (size_t j = 0; j < numTests; j++) {

			

			auto startTime = std::chrono::steady_clock::now();
			vortexes.at(i)->start();
			auto endTime = std::chrono::steady_clock::now();
			vortexes.at(i)->reset();
			
			if (j == 0) {
				first = (endTime - startTime).count();
			}
			sum += (endTime - startTime).count();
			
			times << endTime - startTime << "\n";
			

			
		}

		benchmark << descriptions.at(i) << "\n";
		benchmark << "Average Time: " << sum / numTests << "ns || " << (double)(sum / numTests) / std::pow(10, 9) << "s\n";
		benchmark << "Average speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sum / numTests) / std::pow(10, 9))) << "GB/s\n";

		std::cout << descriptions.at(i) << "\n";
		std::cout << "Average Time: " << sum / numTests << "ns || " << (double)(sum / numTests) / std::pow(10, 9) << "s\n";
		std::cout << "Average speed: " << (vortexes.at(i)->getStreamSizeGB() / ((double)(sum / numTests) / std::pow(10, 9))) << "GB/s\n";
		
	}
	
	
	
	
	
	times.close();
	benchmark.close();
	


}


