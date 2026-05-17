#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <cstdlib>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "bank_common.h"

// Глобальные переменные
BankData* bank = nullptr;
sem_t* sem = nullptr;
int shm_fd;
size_t shm_size;
int server_socket = -1;
volatile bool running = true;  // volatile для сигналов

// Функции для работы с банком
bool validate_account(int account) {
    return account >= 0 && account < bank->num_accounts;
}

void lock() { sem_wait(sem); }
void unlock() { sem_post(sem); }

std::string show_balance(int account) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    lock();
    std::string result = "Balance: " + std::to_string(bank->accounts[account].balance);
    unlock();
    return result;
}

std::string show_min_balance(int account) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    lock();
    std::string result = "Min balance: " + std::to_string(bank->accounts[account].min_balance);
    unlock();
    return result;
}

std::string show_max_balance(int account) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    lock();
    std::string result = "Max balance: " + std::to_string(bank->accounts[account].max_balance);
    unlock();
    return result;
}

std::string freeze_account(int account) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    lock();
    bank->accounts[account].frozen = true;
    unlock();
    return "Account " + std::to_string(account) + " frozen";
}

std::string unfreeze_account(int account) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    lock();
    bank->accounts[account].frozen = false;
    unlock();
    return "Account " + std::to_string(account) + " unfrozen";
}

std::string transfer(int from, int to, int amount) {
    if (amount <= 0) {
        return "Error: Amount must be positive";
    }
    if (!validate_account(from) || !validate_account(to)) {
        return "Error: Invalid account number";
    }
    
    lock();
    
    if (bank->accounts[from].frozen) {
        unlock();
        return "Error: Source account is frozen";
    }
    if (bank->accounts[to].frozen) {
        unlock();
        return "Error: Destination account is frozen";
    }
    
    int new_from = bank->accounts[from].balance - amount;
    if (new_from < bank->accounts[from].min_balance) {
        unlock();
        return "Error: Insufficient funds";
    }
    
    int new_to = bank->accounts[to].balance + amount;
    if (new_to > bank->accounts[to].max_balance) {
        unlock();
        return "Error: Would exceed max balance";
    }
    
    bank->accounts[from].balance = new_from;
    bank->accounts[to].balance = new_to;
    unlock();
    
    return "Success: Transferred " + std::to_string(amount) + 
           " from account " + std::to_string(from) + " to " + std::to_string(to);
}

std::string credit_all(int amount) {
    if (amount <= 0) {
        return "Error: Amount must be positive";
    }
    
    lock();
    for (int i = 0; i < bank->num_accounts; i++) {
        if (!bank->accounts[i].frozen) {
            bank->accounts[i].balance += amount;
        }
    }
    unlock();
    
    return "Success: Credited " + std::to_string(amount) + " to all non-frozen accounts";
}

std::string debit_all(int amount) {
    if (amount <= 0) {
        return "Error: Amount must be positive";
    }
    
    lock();
    for (int i = 0; i < bank->num_accounts; i++) {
        if (!bank->accounts[i].frozen) {
            bank->accounts[i].balance -= amount;
        }
    }
    unlock();
    
    return "Success: Debited " + std::to_string(amount) + " from all non-frozen accounts";
}

std::string set_min_balance(int account, int value) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    
    lock();
    bank->accounts[account].min_balance = value;
    unlock();
    
    return "Success: Min balance for account " + std::to_string(account) + 
           " set to " + std::to_string(value);
}

std::string set_max_balance(int account, int value) {
    if (!validate_account(account)) {
        return "Error: Invalid account number";
    }
    
    lock();
    bank->accounts[account].max_balance = value;
    unlock();
    
    return "Success: Max balance for account " + std::to_string(account) + 
           " set to " + std::to_string(value);
}

std::string process_command(const std::string& cmd) {
    std::vector<std::string> tokens = split(cmd, ' ');
    if (tokens.empty()) {
        return "Error: Empty command";
    }
    
    try {
        if (tokens[0] == "balance" && tokens.size() == 2) {
            return show_balance(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "min" && tokens.size() == 2) {
            return show_min_balance(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "max" && tokens.size() == 2) {
            return show_max_balance(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "freeze" && tokens.size() == 2) {
            return freeze_account(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "unfreeze" && tokens.size() == 2) {
            return unfreeze_account(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "transfer" && tokens.size() == 4) {
            return transfer(std::stoi(tokens[1]), std::stoi(tokens[2]), std::stoi(tokens[3]));
        }
        else if (tokens[0] == "credit" && tokens.size() == 2) {
            return credit_all(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "debit" && tokens.size() == 2) {
            return debit_all(std::stoi(tokens[1]));
        }
        else if (tokens[0] == "set_min" && tokens.size() == 3) {
            return set_min_balance(std::stoi(tokens[1]), std::stoi(tokens[2]));
        }
        else if (tokens[0] == "set_max" && tokens.size() == 3) {
            return set_max_balance(std::stoi(tokens[1]), std::stoi(tokens[2]));
        }
        else if (tokens[0] == "shutdown") {
            return "SHUTDOWN";
        }
        else {
            return "Error: Unknown command";
        }
    } catch (const std::exception& e) {
        return "Error: Invalid number format";
    }
}

void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    delete (int*)arg;
    
    char buffer[4096];
    
    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytes <= 0) {
            break;
        }
        
        std::string command(buffer);
        command.erase(command.find_last_not_of("\n\r") + 1);
        
        if (command.empty()) continue;
        
        std::string response = process_command(command);
        
        // Если команда shutdown
        if (response == "SHUTDOWN") {
            response = "Server shutting down...\n";
            send(client_fd, response.c_str(), response.length(), 0);
            running = false;  // Останавливаем сервер
            close(client_fd);
            break;
        }
        
        response += "\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
    
    close(client_fd);
    return nullptr;
}

bool init_shared_memory() {
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to open shared memory. Run ./bank_init 10 first" << std::endl;
        return false;
    }
    
    struct stat st;
    if (fstat(shm_fd, &st) == -1) {
        return false;
    }
    shm_size = st.st_size;
    
    bank = (BankData*)mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (bank == MAP_FAILED) {
        return false;
    }
    
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        return false;
    }
    
    return true;
}

void signal_handler(int sig) {
    std::cout << "\nReceived signal " << sig << ", shutting down..." << std::endl;
    running = false;
    if (server_socket != -1) {
        close(server_socket);  // Это заставит accept() прерваться
    }
}

int main(int argc, char* argv[]) {
    int port = 8888;
    if (argc > 1) {
        port = std::atoi(argv[1]);
    }
    
    // Обработка сигналов
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (!init_shared_memory()) {
        return 1;
    }
    
    std::cout << "Bank server starting on port " << port << std::endl;
    std::cout << "Bank has " << bank->num_accounts << " accounts" << std::endl;
    std::cout << "Type Ctrl+C or use 'shutdown' command to stop" << std::endl;
    
    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        close(server_socket);
        return 1;
    }
    
    if (listen(server_socket, 10) == -1) {
        std::cerr << "Failed to listen" << std::endl;
        close(server_socket);
        return 1;
    }
    
    std::cout << "Server is ready. Waiting for connections..." << std::endl;
    
    while (running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (running) {
                // Если accept прерван не из-за shutdown
                if (errno != EINTR) {
                    std::cerr << "Accept failed: " << strerror(errno) << std::endl;
                }
            }
            continue;
        }
        
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, INET_ADDRSTRLEN);
        std::cout << "Client connected from " << ip << std::endl;
        
        int* ptr = new int(client_fd);
        pthread_t thread;
        pthread_create(&thread, nullptr, handle_client, ptr);
        pthread_detach(thread);
    }
    
    std::cout << "Waiting for clients to disconnect..." << std::endl;
    sleep(1);  // Даем время клиентским потокам завершиться
    
    close(server_socket);
    munmap(bank, shm_size);
    close(shm_fd);
    sem_close(sem);
    
    std::cout << "Server shutdown complete" << std::endl;
    return 0;
}
