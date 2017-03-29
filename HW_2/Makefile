CC := gcc
#CFLAGS := -O2 -std=gnu99 -Wall
CFLAGS := -O0 -g3 -std=gnu99 -Wall
LFLAGS := -o
INJECT_FLAGS := -shared -fPIC
TEST_SOURCE := hw2_test.c src/test_func.c

TARGET := hw2_test inject.so

.PHONY:clean

all:$(TARGET)

run:$(TARGET)
	LD_PRELOAD=./inject.so ./hw2_test


hw2_test:$(TEST_SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

inject.so:inject.c
	$(CC) -o $@ $(INJECT_FLAGS) $< -ldl



clean:
	rm -f $(TARGET)
