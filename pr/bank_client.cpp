#include <iostream>
#include <string>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <cstring>
#include <cstdlib>
#include "bank_common.h"



class BankClient {

private:
	int shm_fd;
	BankData* bank;
	sem_t* sem;
	int shm_size;

	bool validate_account(int account) {
		return account >=0 && account < bank->num_accounts;
	}

	void lock() {
		sem_wait(sem);
	}

	void unlock() {
		sem_post(sem);
	}

	bool execute_transaction(int from, int to, int amount) {
		if(amount <= 0) {
			std::cout << "Error: amoount must be positive" << std::endl;
			return false;
		}

		if(!validate_account(from) || !validate_account(to)) {
			std::cout << "Error: Invalid account number" << std::endl;
			return false;
		}

		lock();

		if(bank->accounts[from].frozen) {
			std::cout << "Error: source account is frozen" << std::endl;
			unlock();
			return false;
		}

		if(bank->accounts[to].frozen) {
			std::cout << "Error: destination account is frozen" << std::endl;
			unlock();
			return false;	
		}

		int new_from_balance = bank->accounts[from].balance - amount;
		if(new_from_balance < bank->accounts[from].min_balance) {
			std::cout << "Error: Insufficient funds (would violate min balance)" << std::endl;
			unlock();
			return false;
		}

		int new_to_balance = bank->accounts[to].balance + amount;
		if(new_to_balance > bank->accounts[to].max_balance) {
			std::cout << "Error: Transfer would exceed destination max balance" << std::endl;
			unlock();
			return false;
		}

		bank->accounts[from].balance = new_from_balance;
		bank->accounts[to].balance = new_to_balance;
		unlock();

		std::cout << "Success: Transfered " << amount << " from account " << from <<
			" to account " << to << std::endl;
	        return true;	
	}

	void show_balance(int account) {
		if(!validate_account(account)) {
			std::cout << "Error: invalid account number" << std::endl;
			return;
		}

		lock();
		std::cout << "Balance: " << bank->accounts[account].balance << std::endl;
		unlock();
	}

	void show_min_balance(int account) {
		if(!validate_account(account)) {
			std::cout << "Error: invalid account number" << std::endl;
			return;
		}
		
		lock();
		std::cout << "Min balance: " << bank->accounts[account].min_balance << std::endl;
		unlock();
	}

	void show_max_balance(int account) {
		if(!validate_account(account)) {
			std::cout << "Error: invalid account number" << std::endl;
			return;
		}

		lock();
		std::cout << "Max balance: " << bank->accounts[account].max_balance << std::endl;
		unlock();
	}

	void freeze_account(int account) {
		if(!validate_account(account)) {
			std::cout << "Error: invalid account number" << std::endl;
			return;
		}

		lock();
		bank->accounts[account].frozen = true;
		std::cout << "Account " << account << " is frozen" << std::endl;
		unlock();
	}

	void unfreeze_account(int account) {
		if(!validate_account(account)) {
			std::cout << "Error: invalid account number" << std::endl;
			return;
		}

		lock();
		bank->accounts[account].frozen = false;
		std::cout << "Account " << account << " is unfrozen" << std::endl;
		unlock();
	}

	void credit_all(int amount) {
		if(amount <= 0) {
			std::cout << "Error amount must be positive" << std::endl;
			return;
		}

		lock();

		for(int i = 0; i < bank->num_accounts; i++) {
			if(bank->accounts[i].frozen) {
				continue;
			}

			int new_balance = bank->accounts[i].balance + amount;
			if(new_balance > bank->accounts[i].max_balance) {
				std::cout << "Error: Credit operation would exceed max balance on account " 
					<< i << std::endl;
				unlock();
				return;
			}
		}

		for(int i = 0; i < bank->num_accounts; i++) {
			if(!bank->accounts[i].frozen) {
				bank->accounts[i].balance += amount;
			}
		}

		std::cout << "Success: Credit " << amount << " to all non-frozen accounts" << std::endl;
		unlock();
	
	}


	void debit_all(int amount) {
		if(amount <= 0) {
			std::cout << "Error: Amount must be positive " << std::endl;
			return;
		}

		lock();

		for(int i = 0; i < bank->num_accounts; i++) {
			if(bank->accounts[i].frozen) {
				continue;
			}

			int new_balance = bank->accounts[i].balance - amount;
			if(new_balance < bank->accounts[i].min_balance) {
				std::cout << "Error: Debit operation would violate min balance on account " <<
					i << std::endl;

				unlock();
				return;
			}
		}

		for(int i = 0; i < bank->num_accounts; i++) {
			if(!bank->accounts[i].frozen) {
				bank->accounts[i].balance -= amount;
			}
		}

		std::cout << "Success: Debited " << amount << "from all non-frozen accounts" << std::endl;
		unlock();
	
	}


	void set_min_balance(int account, int value) {
		if(!validate_account(account)) {
			std::cout << "Error: Invald account number " << std::endl;
		       return;	
		}

		lock();

		if(value > bank->accounts[account].balance) {
			std::cout << "Error: Min balance cannot exceed current balance" << std::endl;
			unlock();
			return;
		}

		bank->accounts[account].min_balance = value;
		std::cout << "Success: Min balance for account " << account << "has been set to " << value <<
		       std::endl;

		unlock();	
	}

	void set_max_balance(int account, int value) {
		if(!validate_account(account)) {
			std::cout << "Error: Invalid account number" << std::endl;
			return;
		}

		lock();

		if(value < bank->accounts[account].balance) {
			std::cout << "Error: Max balance cannot be less then current balance" << std::endl;
			unlock();
			return;
		}

		bank->accounts[account].max_balance = value;
		std::cout << "Success: Max balance for account " << account << "has been set to " << value <<
			std::endl;
		unlock();
	}


public:
	BankClient(): shm_fd(-1), bank(nullptr), sem(SEM_FAILED) {
		shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
		if(shm_fd == -1) {
			throw std::runtime_error("Failed to open shared memory. Is the bank initialized?");
		}

		struct stat shm_stat;
		if(fstat(shm_fd, &shm_stat) == -1) {
			close(shm_fd);
			throw std::runtime_error("Failed to get shared memory size");
		}
		shm_size = shm_stat.st_size;

		bank = static_cast<BankData*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED,
					shm_fd, 0));
		if(bank == MAP_FAILED) {
			close(shm_fd);
			throw std::runtime_error("Failed to map shared memory");
		}

		sem = sem_open(SEM_NAME, 0);
		if(sem == SEM_FAILED) {
			munmap(bank, shm_size);
			close(shm_fd);
			throw std::runtime_error("Failed to open semaphore");
		}
	}

	~BankClient() {
		if(bank != nullptr && bank != MAP_FAILED) {
			munmap(bank, shm_size);
		}

		if(shm_fd != -1) {
			close(shm_fd);
		}

		if(sem != SEM_FAILED) {
			sem_close(sem);
		}
	
	}

	void run() {
		std::string line;

		while(std::getline(std::cin, line)) {
			if(line.empty()) { continue; }

			std::vector<std::string> tokens = split(line, ' ');
			if(tokens.empty()) continue;

			std::string cmd = tokens[0];

			try {
				if (cmd == "balance" && tokens.size() == 2) {
					int account = std::stoi(tokens[1]);
					show_balance(account);
				}

				else if (cmd == "min" && tokens.size() == 2) {
					int account = std::stoi(tokens[1]);
					show_min_balance(account);
				}

				else if (cmd == "max" && tokens.size() == 2) {
					int acc = std::stoi(tokens[1]);
					show_max_balance(acc);
				}

				else if (cmd == "freeze" && tokens.size() == 2) {
					int acc = std::stoi(tokens[1]);
					freeze_account(acc);
				}

				else if (cmd == "unfreeze" && tokens.size() == 2) {
					int acc = std::stoi(tokens[1]);
					unfreeze_account(acc);
				}

				else if (cmd == "transfer" && tokens.size() == 4) {
					int from = std::stoi(tokens[1]);
					int to = std::stoi(tokens[2]);
					int amount = std::stoi(tokens[3]);
					execute_transaction(from, to, amount);
				}

				else if (cmd == "credit" && tokens.size() == 2) {
					int amount = std::stoi(tokens[1]);
					credit_all(amount);
				}

				else if(cmd == "debit" && tokens.size() == 2) {
					int amount = std::stoi(tokens[1]);
					debit_all(amount);
				}

				else if (cmd == "set_min" && tokens.size() == 3) {
					int account = std::stoi(tokens[1]);
					int amount = std::stoi(tokens[2]);
					set_min_balance(account, amount);
				}

				else if (cmd == "set_max" && tokens.size() == 3) {
					int acc = std::stoi(tokens[1]);
					int value = std::stoi(tokens[2]);
					set_max_balance(acc, value);
				} else {
					std::cout << "Error: Invalid command or incorrect syntax" << std::endl;
				}
			
			
			} catch (const std::exception& e) {
				std::cout << "Error: invalid number format" << std::endl;
			}
		}
	}
};

int main() {

	try {
		BankClient client;
		client.run();

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	
	return 0;

}
