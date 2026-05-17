CXX = g++
CXXFLAGS = -Wall -std=c++11 -pthread
LDFLAGS = -lrt -lpthread

all: bank_init bank_destroy bank_server bank_client

bank_init: bank_init.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_init.cpp -o bank_init $(LDFLAGS)

bank_destroy: bank_destroy.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_destroy.cpp -o bank_destroy $(LDFLAGS)

bank_server: bank_server.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_server.cpp -o bank_server $(LDFLAGS)

bank_client: bank_client.cpp bank_common.h colorprint.h
	$(CXX) $(CXXFLAGS) bank_client.cpp -o bank_client $(LDFLAGS)

clean:
	rm -f bank_init bank_destroy bank_server bank_client

clean-shm:
	@rm -f /dev/shm/shm_transparent_bank /dev/shm/sem.bank_semaphore 2>/dev/null; true

.PHONY: all clean clean-shm
