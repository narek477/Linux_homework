#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string>
#include <iostream>

void PrintWithSysCalls(const std::string& filename) {
	int fd = open(filename.c_str(), O_RDONLY);

	if(fd == -1) {
		perror(("Error cannot open file" + filename).c_str());
		std::exit(EXIT_FAILURE);
	}

	char buffer[4096];
	ssize_t bytesRead;

	while((bytesRead = read(fd, buffer, sizeof(buffer) -1)) > 0) {
		buffer[bytesRead] = '/0';
		printf("%s", buffer);
	}

	if(bytesRead == -1) {
		perror("Error: failed to read file");
		close(fd);
		std::exit(EXIT_FAILURE);
	}
	
	close(fd);


}

int main(int argc, char* argv[]) {
	if(argc != 2) {
		std::cerr << "Error: File path argument is required" << std::endl;
		return EXIT_FAILURE;
	}

	std::string filename = argv[1];

	if(filename.empty()) {
		std::cerr << "Error: empty filename" << std:: endl;
		return EXIT_FAILURE;
	}

	PrintWithSysCalls(filename);

	return EXIT_SUCCESS;
}
