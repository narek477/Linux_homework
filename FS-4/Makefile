CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

all: redirect-cin

redirect-cin: redirect-cin.cpp
	$(CXX) $(CXXFLAGS) -o redirect-cin redirect-cin.cpp

clean:
	rm -f redirect-cin

.PHONY: all clean
