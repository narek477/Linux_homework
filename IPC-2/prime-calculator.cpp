#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include <cstring>
#include <cctype>

bool isValidPositiveInteger(const std::string& s) {
	if(s.empty()) {
		return false;
	}

	for(char c : s) {
		if(!std::isdigit(static_cast<unsigned char>(c))) {
			return false;
		}
	}
	return true;
}

int stringToInt(const std::string& s) {
	int result = 0;
	for(char c : s) {
		result = result * 10 + (c - '0');
	}
	return result;
}


bool isPrime(int n) {
	if(n <= 1) {
		return false;
	}

	for(int i = 2; i * i <=  n; i++) {
		if(n % i == 0) {
			return false;
		}
	}
	return true;
}

int mThPrime(int m) {
	int i = 2;

	while(m > 0) {
		if(isPrime(i)){
			m--;
		}
		i++;
	}
	i -= 1;
	return i;
}


void childProcess(int pipe1[2], int pipe2[2]) {
	close(pipe1[1]);
	close(pipe2[0]);

	while(true) {
		int m;
		int bytes_read = read(pipe1[0], &m, sizeof(m));

		if(bytes_read <= 0) {
			break;
		}
		if(m <= 0) {
			break;
		}

		std::cout << "[Child] calculating " << m << "-th prime number..." << std::endl;
		int result = mThPrime(m);

		std::cout << "[Child] sending calculation resul of prime(" << m << ")..." << std::endl;

		write(pipe2[1], &result, sizeof(result));

	}
	close(pipe1[0]);
	close(pipe2[1]);
	exit(0);
}

void parentProcess(pid_t child_pid, int pipe1[2], int pipe2[2]) {
	close(pipe1[0]);
	close(pipe2[1]);

	std::string input;

	while(true) {
		std::cout << "[Parent] please enter the number: ";
		std::getline(std::cin, input);
	

		if(input == "exit") {
			std::cout << "Exiting the program" << std::endl;
			break;
		}

		if(!isValidPositiveInteger(input)) {
			std::cout << "[Parent] Invalid input " << std::endl;
			continue;
		}

		int m = stringToInt(input);
		if(m <= 0) {
			std::cout << "error" << std::endl;
			continue;
		}

		std::cout << "[Parent] sending" << m <<  "to the child process..." << std::endl;

		write(pipe1[1], &m, sizeof(m));

		std::cout << "[Parent] waiting for the response from the child process" << std::endl;

		int result;
		int bytes_read = read(pipe2[0], &result, sizeof(result));

		if(bytes_read > 0) {
			std::cout << "[Parent] received calculation result of prime(" << m << ")= " << result <<
			"..." << std::endl;
		} else {
			std::cout << "[Parent] error reading result from child" << std::endl;
			break;
		}
	}

	int exit_signal = -1;
	write(pipe1[1], &exit_signal, sizeof(exit_signal));

	close(pipe1[1]);
	close(pipe2[0]);

	waitpid(child_pid, nullptr, 0);

}

int main() {
	int pipe1[2];
	int pipe2[2];

	if(pipe(pipe1) == -1 || pipe(pipe2) == -1) {
		std::cerr << "Pipe creation failed " << std::endl;
		return 1;
	}

	pid_t pid = fork();

	if(pid < 0) {
		std::cerr << "Fork failed" << std::endl;
		return 1;
	}
	
	if(pid == 0) {
		childProcess(pipe1, pipe2);
	} else {
		parentProcess(pid, pipe1, pipe2);
	}

	return 0;
}
