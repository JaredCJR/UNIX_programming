CC := gcc
CPP := g++
#CFLAGS := -O2 -std=gnu99 -Wall -c
#CFLAGS := -O0 -g3 -std=gnu99 -Wall -c
CFLAGS := -O0 -g3 -std=c++17 -Wall -c
LFLAGS := -o

TARGET := hw3
SOURCE := main.cpp
OBJECTS := $(SOURCE).o


all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CPP) $(LFLAGS) $@ $^

gdb:$(TARGET)
	gdb -tui ./$(TARGET)

$(OBJECTS):$(SOURCE)
	$(CPP) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
