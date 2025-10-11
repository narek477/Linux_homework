#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstring>
#include <cstdlib>


int main(int argc, char* argv[]) {
	
	if(argc < 2) {
		std::cerr << "Erro" << std::endl;
		return 1;
	}

	const char* filename = argv[1];

	int fd1 = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(fd1 == -1) {
		std::cerr << "Error cannot open file" << std::endl;
		return -1;
	}

	int fd2 = dup(fd1);
	if(fd2 == -1) {
		std::cerr << "Error cannot dup " << std::endl;
		close(fd1);
		return 1;
	}

	write(fd1, "first line\n", 11);
	write(fd2, "second line\n", 12);

	close(fd1);
	close(fd2);

	return 0;
}
