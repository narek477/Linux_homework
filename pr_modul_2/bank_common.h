#ifndef BANK_COMMON_H
#define BANK_COMMON_H

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <iomanip>

#define SHM_NAME "/transparent_bank"
#define SEM_NAME "/bank_semaphore"
#define DEFAULT_PORT 8888
#define DEFAULT_HOST "127.0.0.1"
#define BUFFER_SIZE 4096

struct Account {
    int balance;
    int min_balance;
    int max_balance;
    bool frozen;
    
    Account() : balance(0), min_balance(0), max_balance(1000000), frozen(false) {}
};

struct BankData {
    int num_accounts;
    Account accounts[0];
};

inline std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

#endif
