NCTU 張家榮 0556054
===============================

- All the Minimum Requirements are included.

Add 5 monitored APIs not included in the Minimum Requirements.
----------------------------------------------------------------
- The following APIs is used inside Ubuntu 16.04 build-in GCC 5.4.0 20160609(Ubuntu 5.4.0-6ubuntu1~16.04.4)
    - GCC does not always call the all following APIs due to the dynamic linkage.
    - If you would like to see all of them being invoked, try to build GCC 5.4 from source code with GCC 5.4
        - int putchar(int);
	    - int puts(const char *);
	    - long ftell(FILE *);
	    - int ungetc(int, FILE *);
	    - pid_t vfork(void);

How to build:
----------------------------------------------------------------

```
make clean
make
```
- cd `the`
- You will find "hw2.so" in the current directory.

Usage Example
----------------------------------------------------------------
```
MONITOR_OUTPUT=stderr LD_PRELOAD=./hw2.so ls -la
```
