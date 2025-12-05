CC = g++
CFLAGS = -Wall -Wextra

TARGET = prime-calculator
SOURCE = prime-calculator.cpp

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)

.PHONY: all clean
