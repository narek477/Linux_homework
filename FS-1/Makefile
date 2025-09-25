CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17 -O2
TARGET := read-file
SOURCE := main.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) *.o

test: all
	@echo "Testing of read-file"
	@./$(TARGET) test1.txt
	@echo "Fisrt test is finished"
	@./$(TARGET) test2.txt
	@echo "Second test is finished"

.PHONY: all clean test


