#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>
#include "bank_common.h"


int main() {
	
	std::cout << std::endl;

	if(shm_unlink(SHM_NAME) == -1) {
		std::cerr << "Warning: Shared memory not found or already removed" << std::endl;
	} else {
		std::cout << "Shared memory removed successfully" << std::endl;
	}

	if(sem_unlink(SEM_NAME) == -1) {
		std::cerr <<  "Warning: Semaphore not found or already removed" << std::endl;
	} else {
		std::cout << "Semaphore removed successfully" << std::endl;
	}

	std::cout << "Bank destroyed successfully" << std::endl;

	return 0;

}
