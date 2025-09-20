CXX = g++

CXXFLAGS = -Wall -Wextra -std=c++17 -g

LDFLAGS =

TARGET = program

SOURCES = main.cpp complex.cpp sort.cpp

OBJECTS = $(SOURCES:.cpp=.o)

HEADRERS = complex.h sort.h

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

main.o: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c main.cpp -o main.o


complex.o: complex.cpp complex.h
	$(CXX) $(CXXFLAGS) -c complex.cpp -o complex.o

sort.o: sort.cpp sort.h
	$(CXX) $(CXXFLAGS) -c sort.cpp -o sort.o

clean:
	rm -f $(OBJECTS) $(TARGET)

run: $(TARGET)
	./$(TARGET)


