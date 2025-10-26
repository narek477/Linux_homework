CXX = g++
CXXFLAGS = -std=c++11 -pthread
TARGET = sumarray
SOURCE = sumarray.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean
