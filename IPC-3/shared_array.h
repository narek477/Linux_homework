#ifndef SHARED_ARRAY_H
#define SHARED_ARRAY_H

#include <string>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


class shared_array {

private:
	int* ptr;
	size_t len;
	int fd;
	std::string mem_name;
public:
	shared_array(const std::string& name, size_t len): len(len) {
		if(len < 1 || len > 1000000000) {
			throw std::runtime_error("Bad size");
		}

		mem_name = "/" + name;
		fd = shm_open(mem_name.c_str(), O_CREAT | O_RDWR, 0666);
		if(fd == -1) {
			throw std::runtime_error("shm_open failed");
		}

		if(ftruncate(fd, len * sizeof(int)) == -1) {
			close(fd);
			throw std::runtime_error("Ftruncate failed");
		}

		ptr = (int*)mmap(NULL, len * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if(ptr == MAP_FAILED) {
			shm_unlink(mem_name.c_str());
			perror("mmap");
			close(fd);
			
		}
	
	}

	int& operator[](int i) {
		if(i >= len) {
			throw std::out_of_range("Too big");
		}
		return ptr[i];
	}

	size_t size() const {
		return len;
	}
        
        ~shared_array() {
		if(ptr != MAP_FAILED) {
			munmap(ptr, len * sizeof(int));
		}
		
		close(fd);
	}	

};

#endif
