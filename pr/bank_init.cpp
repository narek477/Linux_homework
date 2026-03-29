#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include "bank_common.h"


int main(int argc, char* argv[]) {
	
	if(argc != 2) {
		std::cerr << "number of accounts" << std::endl;
		return 1;
	}

	int n = std::atoi(argv[1]);
	if(n <= 0) {
		std::cerr << "Number of accounts needs to be positive" << std::endl;
		return 1;
	}

	int shm_size = sizeof(BankData) + n * sizeof(Account);

	int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
	if(shm_fd == -1) {
		std::cerr << "Failed to create shared memry" << std::endl;
		return 1;
	}

	if(ftruncate(shm_fd, shm_size) == -1) {
		std::cerr << "failed to set shared memory size" << std::endl;
		close(shm_fd);
		shm_unlink(SHM_NAME);
		return 1;
	}

	void* ptr = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if(ptr == MAP_FAILED) {
		std::cerr << "Failed to map shared memory" << std::endl;
		close(shm_fd);
		shm_unlink(SHM_NAME);
		return 1;
	}

	BankData* bank = static_cast<BankData*>(ptr);
	bank->num_accounts = n;
	for(int i = 0; i < n; i++) {
		new (&bank->accounts[i]) Account();
	}

	sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
	if(sem == SEM_FAILED) {
		std::cerr << "Failed to cerate semaphore" << std::endl;
		munmap(ptr, shm_size);
		close(shm_fd);
		shm_unlink(SHM_NAME);
		return 1;
	}

	std::cout << "Bank initialised with" << n << "accounts." << std::endl;

	munmap(ptr, shm_size);
	close(shm_fd);
	sem_close(sem);

	return 0;

}
