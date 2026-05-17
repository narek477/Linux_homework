#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <signal.h>
#include <cstdlib>
#include "bank_common.h"

// Глобальные переменные для сервера
BankData* bank = nullptr;
sem_t* sem = nullptr;
int shm_fd;
size_t shm_size;
int server_socket = -1;
volatile bool running = true;

// Статистика запросов
int request_count = 0;
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t stats_cond = PTHREAD_COND_INITIALIZER;
bool shutdown_server = false;

// Функции для работы с банком (те же, что и в клиенте)
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
    
    int new_from_balance = bank->accounts[from].balance - amount;
    if (new_from_balance < bank->accounts[from].min_balance) {
        unlock();
        return "Error: Insufficient funds (would violate min balance)";
    }
    
    int new_to_balance = bank->accounts[to].balance + amount;
    if (new_to_balance > bank->accounts[to].max_balance) {
        unlock();
        return "Error: Transfer would exceed destination max balance";
    }
    
    bank->accounts[from].balance = new_from_balance;
    bank->accounts[to].balance = new_to_balance;
    unlock();
    
    return "Success: Transferred " + std::to_string(amount) + 
           " from account " + std::to_string(from) + " to account " + std::to_string(to);
}

std::string credit_all(int amount) {
    if (amount <= 0) {
        return "Error: Amount must be positive";
    }
    
    lock();
    
    for (int i = 0; i < bank->num_accounts; i++) {
        if (bank->accounts[i].frozen) continue;
        if (bank->accounts[i].balance + amount > bank->accounts[i].max_balance) {
            unlock();
            return "Error: Credit operation would exceed max balance on account " + std::to_string(i);
        }
    }
    
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
        if (bank->accounts[i].frozen) continue;
        if (bank->accounts[i].balance - amount < bank->accounts[i].min_balance) {
            unlock();
            return "Error: Debit operation would violate min balance on account " + std::to_string(i);
        }
    }
    
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
    if (value > bank->accounts[account].balance) {
        unlock();
        return "Error: Min balance cannot exceed current balance";
    }
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
    if (value < bank->accounts[account].balance) {
        unlock();
        return "Error: Max balance cannot be less than current balance";
    }
    bank->accounts[account].max_balance = value;
    unlock();
    
    return "Success: Max balance for account " + std::to_string(account) + 
           " set to " + std::to_string(value);
}

// Обработка команды от клиента
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
            return "Error: Unknown command or incorrect syntax";
        }
    } catch (const std::exception& e) {
        return "Error: Invalid number format";
    }
}

// Функция для вывода статистики (запускается в отдельной нити)
void* stats_printer(void* arg) {
    pthread_mutex_lock(&stats_mutex);
    while (!shutdown_server) {
        pthread_cond_wait(&stats_cond, &stats_mutex);
        if (shutdown_server) break;
        std::cout << "[STATS] Total requests processed: " << request_count << std::endl;
    }
    pthread_mutex_unlock(&stats_mutex);
    return nullptr;
}

// Функция для обработки клиента (в отдельной нити)
void* handle_client(void* arg) {
    int client_fd = *(int*)arg;
    delete (int*)arg;
    
    char buffer[BUFFER_SIZE];
    std::string response;
    
    while (running && !shutdown_server) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_read <= 0) {
            break;
        }
        
        std::string command(buffer);
        command.erase(command.find_last_not_of("\n\r") + 1);
        
        if (command.empty()) continue;
        
        // Обрабатываем команду
        response = process_command(command);
        
        // Обновляем статистику
        pthread_mutex_lock(&stats_mutex);
        request_count++;
        if (request_count % 5 == 0) {
            pthread_cond_signal(&stats_cond);
        }
        pthread_mutex_unlock(&stats_mutex);
        
        // Проверяем shutdown
        if (response == "SHUTDOWN") {
            response = "Server is shutting down...";
            send(client_fd, response.c_str(), response.length(), 0);
            shutdown_server = true;
            running = false;
            close(client_fd);
            break;
        }
        
        response += "\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
    
    close(client_fd);
    return nullptr;
}

// Инициализация разделяемой памяти
bool init_shared_memory() {
    shm_fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm_fd == -1) {
        std::cerr << "Failed to open shared memory. Run bank_init first." << std::endl;
        return false;
    }
    
    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        std::cerr << "Failed to get shared memory size" << std::endl;
        return false;
    }
    shm_size = shm_stat.st_size;
    
    bank = static_cast<BankData*>(mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (bank == MAP_FAILED) {
        std::cerr << "Failed to map shared memory" << std::endl;
        return false;
    }
    
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        std::cerr << "Failed to open semaphore" << std::endl;
        munmap(bank, shm_size);
        return false;
    }
    
    return true;
}

void cleanup() {
    if (bank && bank != MAP_FAILED) {
        munmap(bank, shm_size);
    }
    if (shm_fd != -1) {
        close(shm_fd);
    }
    if (sem != SEM_FAILED) {
        sem_close(sem);
    }
    if (server_socket != -1) {
        close(server_socket);
    }
    pthread_mutex_destroy(&stats_mutex);
    pthread_cond_destroy(&stats_cond);
}

void signal_handler(int sig) {
    std::cout << "\nReceived signal " << sig << ", shutting down..." << std::endl;
    running = false;
    shutdown_server = true;
    pthread_cond_signal(&stats_cond);
}

int main(int argc, char* argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) {
        port = std::atoi(argv[1]);
        if (port <= 0 || port > 65535) {
            std::cerr << "Invalid port number" << std::endl;
            return 1;
        }
    }
    
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    if (!init_shared_memory()) {
        return 1;
    }
    
    std::cout << "Bank server starting on port " << port << std::endl;
    std::cout << "Bank has " << bank->num_accounts << " accounts" << std::endl;
    
    // Создаем нить для вывода статистики
    pthread_t stats_thread;
    if (pthread_create(&stats_thread, nullptr, stats_printer, nullptr) != 0) {
        std::cerr << "Failed to create stats thread" << std::endl;
        cleanup();
        return 1;
    }
    
    // Создаем сокет
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        cleanup();
        return 1;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Failed to bind socket" << std::endl;
        cleanup();
        return 1;
    }
    
    if (listen(server_socket, 10) == -1) {
        std::cerr << "Failed to listen on socket" << std::endl;
        cleanup();
        return 1;
    }
    
    std::cout << "Server listening for connections..." << std::endl;
    
    // Основной цикл принятия клиентов
    while (running && !shutdown_server) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_fd = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (running) {
                std::cerr << "Failed to accept connection" << std::endl;
            }
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Client connected from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        
        int* client_fd_ptr = new int(client_fd);
        pthread_t client_thread;
        if (pthread_create(&client_thread, nullptr, handle_client, client_fd_ptr) != 0) {
            std::cerr << "Failed to create client thread" << std::endl;
            delete client_fd_ptr;
            close(client_fd);
        } else {
            pthread_detach(client_thread);
        }
    }
    
    std::cout << "Waiting for all clients to disconnect..." << std::endl;
    sleep(1);
    
    cleanup();
    std::cout << "Server shutdown complete" << std::endl;
    
    return 0;
}
