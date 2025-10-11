CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: manual-append

manual-append: manual-append.cpp
	$(CXX) $(CXXFLAGS) -o manual-append manual-append.cpp

clean:
	rm -f manual-append

.PHONY: all clean
