#include <iostream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>

struct tdata {
	
	long long sum;
	int start;
	int end;
	long long* arr;

};

void* partial_sum(void* arg) {
	tdata* data = (tdata*) arg;
	data->sum  = 0;

	for(int i = data->start; i < data->end; i++) {
		data->sum += data-> arr[i];
	}

	return nullptr;
}


long long sum_array(long long* arr, int n, int num_threads) {
	if(num_threads == 1) {
		long long sum = 0; 
		for(int i = 0; i < n; i++) {
			sum += arr[i];
		}
		return sum;
	} else {
		pthread_t threads[num_threads];
		tdata thread_data[num_threads];
		int chunk = n / num_threads;
		long long totalsum = 0;

		for(int i = 0; i < num_threads; i++) {
			thread_data[i].arr = arr;
			thread_data[i].start = i * chunk;
			thread_data[i].end = (i == num_threads - 1) ? n : (i + 1) * chunk;

			if(pthread_create(&threads[i], nullptr, partial_sum, &thread_data[i]) != 0) {
				std::cerr << "Thread creation failed" << std::endl;
			
				return 1;
			}
		}
	
		for(int i = 0; i < num_threads; i++) {
			pthread_join(threads[i], nullptr);
			totalsum += thread_data[i].sum;
		}
		return totalsum;
	}

}


int main(int argc, char* argv[]) {

	if(argc != 3) {
		std::cout << "Error" << std::endl;
	}

	int N = atoi(argv[1]);
	int M = atoi(argv[2]);

	long long* arr = new long long[N];
	srand(time(NULL));
	for(int i = 0; i < N; i++) {
		arr[i] = rand() % 1000 + 1;
	}

	clock_t start = clock();
	long long res1 = sum_array(arr, N, 1);
	clock_t end = clock();
	double time1 = (double)(end - start) / CLOCKS_PER_SEC * 1000000;

	start = clock();
	long long res2 = sum_array(arr, N, M);
	end = clock();
	double time2 = (double)(end - start) / CLOCKS_PER_SEC * 1000000;

	std::cout << "Time spend without threads: " << time1 << std::endl;
	std::cout << "Time spend with M threads: " << time2 << std::endl;

	delete[] arr;
	return 0;
}
