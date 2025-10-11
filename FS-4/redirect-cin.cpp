#include <iostream>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>


void initialise(int argc, char** argv) {

	if(argc < 2) {
		std::cerr << "Error" << std::endl;
		std::exit(1);
	}

	const char* filename = argv[1];

	int fd = open(filename, O_RDONLY);
	if(fd == -1) {
		std::cerr << "Error cannot open file" << std::endl;
		std::exit(errno);
	}

	dup2(fd, 0);

	close(fd);

}


int main(int argc, char** argv) {
	
	initialise(argc, argv);

	std::string input;
	std::cin >> input;

	std::string reversed;

	for(int i = input.length() - 1; i >= 0; i--) {
		reversed += input[i];
	}

	std::cout << reversed << std::endl;

	return 0;
	
}





