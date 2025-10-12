#include <iostream>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>


void do_command(char** argv) {
	if(argv == nullptr || argv[0] == nullptr) {
	
		std::cerr << "Error no command specified" << std::endl;
		return;
	}

	time_t start_time = time(nullptr);
	
	pid_t pid = fork();

	if(pid == -1) {
		std::cerr << "Error failed to create process" << std::endl;
		return;
	} else if(pid == 0) {
		execvp(argv[0], argv);
		std::cerr << "error failed to execute command" << std::endl;
		exit(EXIT_FAILURE);
	} else {
		int status;
		wait(&status);

		time_t end_time = time(nullptr);
		double duration = difftime(end_time, start_time);

		if(WIFEXITED(status)) {
			std::cout << "Command completed with " << WEXITSTATUS(status) << "exit conde and took "
				<< duration << " seconds." << std::endl;
		}
	}

}


int main(int argc, char** argv) {
	if(argc < 2) {
		return 1;
	}

	do_command(argv + 1);

	return 0;

}




