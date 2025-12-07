CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra
TARGET = echo-signal

all: $(TARGET)

$(TARGET): main1.cpp
	$(CXX) $(CXXFLAGS) -o $(TARGET) main1.cpp

clean:
	rm -f $(TARGET)

.PHONY: all clean
