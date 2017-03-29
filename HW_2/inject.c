#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>

static uid_t (*old_getuid)(void) = NULL;
static int (*old_fputc)( int character, FILE * stream ) = NULL;

#define get_old_name(name) old_ ## name

#define __get_old_func(old_p2func,orig_func_name)  do{ \
	if(old_p2func == NULL) { \
		void *handle = dlopen("libc.so.6", RTLD_LAZY); \
		if(handle != NULL) { \
			old_p2func = dlsym(handle, #orig_func_name); \
		} \
	} \
}while(0)

#define get_old_func(orig_name) __get_old_func(get_old_name(orig_name),orig_name)

#define return_old_func(orig_name,...)  do{ \
	if(get_old_name(orig_name) != NULL) { \
	    return get_old_name(orig_name)(__VA_ARGS__); \
	}else \
    { \
        fprintf(stderr,"Get original library call error!\n"); \
    } \
}while(0)

#define get_orig_and_ret(orig_name,...) do{\
    get_old_func(orig_name); \
    return_old_func(orig_name,__VA_ARGS__); \
}while(0)

uid_t getuid(void) {
    get_orig_and_ret(getuid);
    return 0;
}

int fputc(int character, FILE * stream )
{
    get_orig_and_ret(fputc,character,stream);
    return 0;
}

