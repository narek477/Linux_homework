#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <fcntl.h>

int main() {
	
	std::string input;
	while(true) {
		getline(std::cin, input);

		if(input == "exit") {
			break;
		}

		if(input.empty()) {
			continue;
		}

		bool silent = false;
		std::string com = input;

		if(input.find("silent ") == 0) {
			silent == true;
			com = input.substr(7);
		}

		pid_t pid = fork();

		if(pid == 0) {
			if(silent) {
				std::string filename = std::to_string(getpid()) + ".log";
				int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
				if(fd == -1) {
					perror("open");
					exit(1);
				}
				dup2(fd, 1);
				dup2(fd, 2);
				close(fd);
			}

			system(com.c_str());
			exit(0);
		
		} else if(pid > 0) {
			wait(NULL);
		
		} else {
			std::cout << "Failed to create process" << std::endl;

		}
	
	}

	return 0;

}
