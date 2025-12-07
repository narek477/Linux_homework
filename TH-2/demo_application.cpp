#include <iostream>
#include <unistd.h>
#include "parallel_scheduler.h"


void increment_and_print(void* arg) {
	int* value = static_cast<int*>(arg);
	int result = (*value) + 1;
	std::cout << "Thread " << pthread_self() << ": " << *value << "+ 1 = " << result << std::endl;

	usleep(300000);
	delete value;
}


int main() {
	parallel_scheduler scheduler(3);

	std::cout << "Thread pool with 3 workers created" << std::endl;
	std::cout << "Adding 8 tasks to the queue" << std::endl;

	for(int i = 1; i <= 8; i++) {
		int* number = new int(i);
		scheduler.execute(increment_and_print, number);
	}


	usleep(3000000);

	std::cout << "all tasks completed" << std::endl;
	return 0;
}




