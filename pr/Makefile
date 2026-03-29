CXX = g++
CXXFLAGS = -Wall -std=c++11
LDFLAGS = -lrt -lpthread

all: bank_init bank_client bank_destroy

bank_init: bank_init.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_init.cpp -o bank_init $(LDFLAGS)

bank_client: bank_client.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_client.cpp -o bank_client $(LDFLAGS)

bank_destroy: bank_destroy.cpp bank_common.h
	$(CXX) $(CXXFLAGS) bank_destroy.cpp -o bank_destroy $(LDFLAGS)

test: all
	@./bank_init 5
	@echo "\n=== TEST 1: Basic operations ==="
	@./bank_client < test1_basic.txt
	@echo "\n=== TEST 2: Error handling ==="
	@./bank_client < test2_errors.txt
	@echo "\n=== TEST 3: Freeze/Unfreeze ==="
	@./bank_client < test3_freeze.txt
	@echo "\n=== TEST 4: Min/Max limits ==="
	@./bank_client < test4_limits.txt
	@echo "\n=== TEST 5: Bulk operations ==="
	@./bank_client < test5_bulk_operations.txt
	@echo "\n=== TEST 6: Mixed operations ==="
	@./bank_client < test6_mixed.txt
	@./bank_destroy

clean:
	rm -f bank_init bank_client bank_destroy

clean-shm:
	rm -f /dev/shm/shm_transparent_bank /dev/shm/sem.bank_semaphore 2>/dev/null; true


.PHONY: all test clean clean-shm


