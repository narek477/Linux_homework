CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17
TARGET = simple-copy
SRC = simple-copy.cpp

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

all: $(TARGET)

clean: 
	rm -f $(TARGET)

test: $(TARGET)
	@echo "Creating the test file"
	@echo "Hey I am inside the test file" > test-source.txt
	./$(TARGET) test-source.txt test-destination.txt
	@echo "Testing the result"
	cat test-destination.txt

.PHONY: all clean test

