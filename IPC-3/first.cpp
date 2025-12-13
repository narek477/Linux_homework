#include <iostream>
#include <semaphore.h>
#include <unistd.h>
#include "shared_array.h"


sem_t* get_sem(const std::string& name) {
	std::string sem_name = "/" + name + "_sem";
	return sem_open(sem_name.c_str(), O_CREAT, 0666, 1);
}

int main() {
	shared_array arr("arr", 5);
	sem_t* sem = get_sem("arr");

	if(sem == SEM_FAILED) {
		std::cerr << "Semaphore error\n";
		return 1;
	}

	int count = 0;
	while(true) {
		sem_wait(sem);

		for(int i = 0; i < arr.size(); i++) {
			arr[i] = count + i;
		}

		std::cout << "Processs 1 : ";
		for(int i = 0; i < arr.size(); i++) {
			std::cout << arr[i] << " ";
		}
		

		sem_post(sem);

	
		sleep(1);

	}

	sem_close(sem);
	return 0;


}
