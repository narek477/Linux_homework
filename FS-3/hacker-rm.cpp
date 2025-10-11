#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>

int main(int argc, char* argv[]) {

	if(argc < 2) {
		return 1;
	}

	const char* filename = argv[1];

	int file = open(filename, O_RDWR);
	if(file == -1) {
		std::cerr << "Error cannot open file" << std::endl;
		return 1;
	}

	int filesize = lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);


	for(int i = 0; i < filesize; i++) {
		write(file, "/0", 1);
	}
	
	close(file);
	unlink(filename);
	return 0;

}
