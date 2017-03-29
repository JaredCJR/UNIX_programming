CC := gcc
#CFLAGS := -O2 -std=gnu99 -Wall
CFLAGS := -O0 -g3 -std=gnu99 -Wall
LFLAGS := -o
INJECT_FLAGS := -shared -fPIC


TARGET := hw2_test inject.so

.PHONY:clean

all:$(TARGET)

run:$(TARGET)
	LD_PRELOAD=./inject.so ./hw2_test


hw2_test:hw2_test.c
	$(CC) $(CFLAGS) $^ -o $@

inject.so:inject.c
	$(CC) -o $@ $(INJECT_FLAGS) $< -ldl



clean:
	rm -f $(TARGET)
