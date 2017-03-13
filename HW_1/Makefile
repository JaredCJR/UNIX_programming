CC := gcc
CFLAGS := -O2 -std=gnu99 -Wall -c
#CFLAGS := -O0 -g3 -std=gnu99 -Wall -c
LFLAGS := -o

TARGET := hw1
SOURCE := main.c
OBJECTS := $(SOURCE).o


all:$(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) $(LFLAGS) $@ $^


$(OBJECTS):$(SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)
