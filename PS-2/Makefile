CXX = g++
CXXFLAGS = -std=c++11 -Wall
TARGET = intershell
SOURCE = intershell.cpp


all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET) *.log

.PHONY: all clean
