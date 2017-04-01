CC := gcc
CFLAGS := -O2 -std=gnu99 -Wall
#CFLAGS := -O0 -g3 -std=gnu99 -Wall
LFLAGS := -o
INJECT_FLAGS := -shared -fPIC
TEST_SOURCE := hw2_test.c src/test_func.c
PRELOAD_NAME := hw2.so
TARGET := hw2_test $(PRELOAD_NAME)

.PHONY:clean

all:$(TARGET)
	strip --strip-unneeded $(PRELOAD_NAME)
	objcopy -R .comment -R .note.ABI-tag -R .gnu.version $(PRELOAD_NAME)

run:$(TARGET)
	LD_PRELOAD=./$(PRELOAD_NAME) ./hw2_test

gdb:$(TARGET)
	LD_PRELOAD=./$(PRELOAD_NAME) gdbtui ./hw2_test

hw2_test:$(TEST_SOURCE)
	$(CC) $(CFLAGS) $^ -o $@

$(PRELOAD_NAME):inject.c
	$(CC) -O0 -std=gnu99 -o $@ $(INJECT_FLAGS) $< -ldl



clean:
	rm -f $(TARGET) monitor.out
