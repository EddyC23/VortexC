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

void testproducer0(const void* bufWptr) {

	ULONGLONG i = 0;

	while (i < 1ULL << 20 >> 2) {
		((int*)bufWptr)[i] = i;
		
		i++;
	}
	Vortex::producer_done();
	
}

void testconsumer0(const void* bufRptr) {
	ULONGLONG i = 0;

	long long sum = 0;
	while (i < 1ULL<< 20 >>2) {
		sum += ((int*)bufRptr)[i];
		i++;
	}
}

void testproducer1(const void* bufWptr) {

	ULONGLONG i = 0;

	while (i < (1ULL << 20) - 3) {
		
		
		memset((void *)((ULONG_PTR)bufWptr + i), 'a', 3);
		i += 3;
	}
	
	((char*)bufWptr)[i] = 'z';
	Vortex::producer_done();
}

void testconsumer1(const void* bufRptr) {
	ULONGLONG i = 0;
	
	while (i < 1ULL<<20) {
		if (((char*)bufRptr)[i]) {}
		i++;
	}
}

void testproducer2(const void* bufWptr) {

	ULONGLONG i = 0;

	while (i < 1ULL << 27 >> 2) {
		((int*)bufWptr)[i] = i;
		i++;
	}
	Vortex::producer_done();
}

void testconsumer2(const void* bufRptr) {
	ULONGLONG i = 0;

	long long sum = 0;
	while (i < 1ULL << 27 >> 2) {
		sum += ((int*)bufRptr)[i];
		i++;
	}
}

void testproducer3(const void* bufWptr) {

	ULONGLONG i = 0;

	while (i < (1ULL << 27) - 3) {


		memset((void*)((ULONG_PTR)bufWptr + i), 'a', 3);
		i += 3;
	}

	((char*)bufWptr)[i] = 'z';
	Vortex::producer_done();
}

void testconsumer3(const void* bufRptr) {
	ULONGLONG i = 0;

	while (i < 1ULL << 27) {
		if (((char*)bufRptr)[i]) {}
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
	
	
	
	

	ULONGLONG STREAM_SIZE_POWER, BLOCK_SIZE_PAGE_POWER;
	unsigned int L, M, N;
	
	STREAM_SIZE_POWER = 20, BLOCK_SIZE_PAGE_POWER = 0;
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

	std::ofstream output("Benchmark.txt", std::ios_base::out);
	std::vector<std::string> descriptions =
	{
		"1mb, 1 page blocks, L = 2, M = 3, N = 4, int producer, summation consumer\n",
		"1mb, 8 page blocks, L = 2, M = 3, N = 4, int producer, summation consumer\n"
		"1mb, 1 page blocks, L = 2, M = 3, N = 4, char producer, plain consumer\n",
		"1mb, 8 page blocks, L = 2, M = 3, N = 4, char producer, plain consumer\n"
		"128mb, 1 page blocks, L = 2, M = 3, N = 4, int producer, summation consumer\n"
		"128mb, 8 page blocks, L = 2, M = 3, N = 4, int producer, summation consumer\n"
		"128mb, 1 page blocks, L = 2, M = 3, N = 4, char producer, plain consumer\n"
		"128mb, 8 page blocks, L = 2, M = 3, N = 4, char producer, plain consumer\n"

	};
	std::vector<Vortex * > vortexes = { &v0,&v1,&v2,&v3,&v4,&v5,&v6,&v7 };
	

	for (size_t i = 0; i < 8; i++) {

		output << descriptions.at(i);

		ULONGLONG sum = 0;

		for (size_t j = 0; j < 30; j++) {

			

			auto startTime = std::chrono::steady_clock::now();
			vortexes.at(i)->start();
			auto endTime = std::chrono::steady_clock::now();
			vortexes.at(i)->reset();

			sum += (endTime - startTime).count();

			output << endTime - startTime << "\n";
		}
		output << "Average Time: " << sum / 30 << "ns\n\n";
	}
	
	
	

	output.close();


}


