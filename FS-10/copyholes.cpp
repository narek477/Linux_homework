#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <cstdlib>


int main(int argc, char** argv) {
	if(argc < 3) {
		std::cerr << "error in the input" << std::endl;
		return 1;
	}

	const char* source_path = argv[1];
	const char* dest_path = argv[2];

	int source_fd = open(source_path, O_RDONLY);
	if(source_fd == -1) {
		std::cerr << "Error cannot open source file" << std::endl;
		return 1;
	}

	int dest_fd = open(dest_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if(dest_fd == -1) {
		std::cerr << "Error cannot create dest file" << std::endl;
		return 1;
	}

	off_t filesize = lseek(source_fd, 0, SEEK_END);
	lseek(source_fd, 0, SEEK_SET);

	const size_t buffersize = 4096;
	char buffer[buffersize];

	long long data = 0;
	off_t curr_pos = 0;

	while(curr_pos < filesize) {
		ssize_t bytes_read = read(source_fd, buffer, buffersize);

		if(bytes_read == -1) {
			std::cerr << "Error reading file" << std::endl;
			break;
		}

		if(bytes_read == 0) {
			break;
		}

		ssize_t bytes_written = write(dest_fd, buffer, bytes_read);
		if(bytes_written == -1) {
			std::cerr << "Error writing file" << std::endl;
			break;
		}

		data += bytes_written;
		curr_pos += bytes_read;
	
	}

	long long hole_bites = filesize - data;

	close(source_fd);
	close(dest_fd);

	std::cout << "Successfull copied " << filesize << "bytes(data: " << data << ", hole: "
		<< hole_bites << ")." << std::endl;

	return 0;


}
