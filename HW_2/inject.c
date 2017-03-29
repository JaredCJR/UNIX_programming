#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>


#ifndef __O_TMPFILE
    #define __O_TMPFILE     020000000
#endif
/*gcc bug*/
#ifndef O_TMPFILE
    #define O_TMPFILE (__O_TMPFILE | O_DIRECTORY)
#endif

#ifndef EXIT_FAILURE
	#define EXIT_FAILURE 1
#endif

static int (*old_fputc)( int character, FILE * stream ) = NULL;
static int (*old_closedir)(DIR *dirp) = NULL;
static DIR *(*old_fdopendir)(int fd) = NULL;
static DIR *(*old_opendir)(const char *name) = NULL;
static struct dirent *(*old_readdir)(DIR *dirp) = NULL;
static int (*old_readdir_r)(DIR *dirp, struct dirent *entry, struct dirent **result) = NULL;
static void (*old_rewinddir)(DIR *dirp) = NULL;
static void (*old_seekdir)(DIR *dirp, long loc) = NULL;
static long (*old_telldir)(DIR *dirp) = NULL;
static int (*old_creat)(const char *pathname, mode_t mode) = NULL;
static int (*old_open)(const char *path, int oflag, ...) = NULL;
static int (*old_fclose)(FILE *stream) = NULL;
static FILE *(*old_fdopen)(int fd, const char *mode) = NULL;
static int (*old_fflush)(FILE *stream) = NULL;
static int (*old_fgetc)(FILE *stream) = NULL;
static char *(*old_fgets)(char *s, int size, FILE *stream) = NULL;
static FILE *(*old_fopen)(const char *path, const char *mode) = NULL;
static int (*old_fputs)(const char *s, FILE *stream) = NULL;
static size_t (*old_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
static size_t (*old_fwrite)(const void *ptr, size_t size, size_t nmemb,FILE *stream) = NULL;
static int (*old_remove)(const char *pathname) = NULL;
static int (*old_rename)(const char *old, const char *new) = NULL;
static void (*old_setbuf)(FILE *stream, char *buf) = NULL;
static int (*old_setvbuf)(FILE *stream, char *buf, int mode, size_t size) = NULL;
static char *(*old_tempnam)(const char *dir, const char *pfx) = NULL;
static FILE *(*old_tmpfile)(void) = NULL;
static char *(*old_tmpnam)(char *s) = NULL;
static void *(*old_calloc)(size_t nmemb, size_t size) = NULL;
static void (*old_exit)(int status) = NULL;
static void (*old_free)(void *ptr) = NULL;
static char *(*old_getenv)(const char *name) = NULL;
static void *(*old_malloc)(size_t size) = NULL;
static char *(*old_mkdtemp)(char *template) = NULL;
static int (*old_mkstemp)(char *template) = NULL;
static int (*old_putenv)(char *string) = NULL;
static int (*old_rand)(void) = NULL;
static int (*old_rand_r)(unsigned int *seedp) = NULL;
static void *(*old_realloc)(void *ptr, size_t size) = NULL;
static int (*old_setenv)(const char *name, const char *value, int overwrite) = NULL;
static void (*old_srand)(unsigned int seed) = NULL;
static int (*old_system)(const char *command) = NULL;
static int (*old_chdir)(const char *path) = NULL;
static int (*old_chown)(const char *path, uid_t owner, gid_t group) = NULL;
static int (*old_close)(int fildes) = NULL;
static int (*old_dup)(int fildes) = NULL;
static int (*old_dup2)(int fildes, int fildes2) = NULL;
static void (*old__exit)(int status) = NULL;
static int (*old_execl)(const char *path, const char *arg, ...) = NULL;
static int (*old_execle)(const char *path, const char *arg, ...) = NULL;
static int (*old_execlp)(const char *file, const char *arg, ...) = NULL;
static int (*old_execv)(const char *path, char *const argv[]) = NULL;
static int (*old_execve)(const char *path, char *const argv[], char *const envp[]) = NULL;
static int (*old_execvp)(const char *file, char *const argv[]) = NULL;
static int (*old_execvpe)(const char *file, char *const argv[],char *const envp[]) = NULL;
static int (*old_fchdir)(int fildes) = NULL;
static int (*old_fchown)(int fildes, uid_t owner, gid_t group) = NULL;
static pid_t (*old_fork)(void) = NULL;
static int (*old_fsync)(int fildes) = NULL;
static int (*old_ftruncate)(int fildes, off_t length) = NULL;
static char *(*old_getcwd)(char *buf, size_t size) = NULL;
static gid_t (*old_getegid)(void) = NULL;
static uid_t (*old_geteuid)(void) = NULL;
static gid_t (*old_getgid)(void) = NULL;
static uid_t (*old_getuid)(void) = NULL;
static int (*old_link)(const char *path1, const char *path2) = NULL;
static int (*old_pipe)(int fildes[2]) = NULL;
static ssize_t (*old_pread)(int fildes, void *buf, size_t nbyte, off_t offset) = NULL;
static ssize_t (*old_pwrite)(int fildes, const void *buf, size_t nbyte,off_t offset) = NULL;
static ssize_t (*old_read)(int fildes, void *buf, size_t nbyte) = NULL;
static ssize_t (*old_readlink)(const char *restrict path, char *restrict buf,size_t bufsize) = NULL;
static int (*old_rmdir)(const char *path) = NULL;
static int (*old_setegid)(gid_t gid) = NULL;
static int (*old_seteuid)(uid_t uid) = NULL;
static int (*old_setgid)(gid_t gid) = NULL;
static int (*old_setuid)(uid_t uid) = NULL;
static unsigned int (*old_sleep)(unsigned int seconds) = NULL;
static int (*old_symlink)(const char *path1, const char *path2) = NULL;
static int (*old_unlink)(const char *path) = NULL;
static ssize_t (*old_write)(int fildes, const void *buf, size_t nbyte) = NULL;


#define get_old_name(name) old_ ## name

#define __get_old_func(old_p2func,orig_func_name)  do{ \
	if(old_p2func == NULL) { \
		void *handle = dlopen("libc.so.6", RTLD_LAZY); \
		if(handle != NULL) { \
			old_p2func = dlsym(handle, #orig_func_name); \
		}else \
        {  \
            fprintf(stderr,"handle is NULL\n"); \
        }  \
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


uid_t getuid(void)
{
    get_orig_and_ret(getuid);
    return EXIT_FAILURE;
}

int fputc(int character, FILE * stream )
{
    get_orig_and_ret(fputc,character,stream);
    return EXIT_FAILURE;
}

int closedir(DIR *dirp)
{
    get_orig_and_ret(closedir,dirp);
    return EXIT_FAILURE;
}

DIR *fdopendir(int fd)
{
    get_orig_and_ret(fdopendir,fd);
    return NULL;
}

DIR *opendir(const char *name)
{
    get_orig_and_ret(opendir,name);
    return NULL;
}

struct dirent *readdir(DIR *dirp)
{
    get_orig_and_ret(readdir,dirp);
    return NULL;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
    get_orig_and_ret(readdir_r,dirp,entry,result);
    return EXIT_FAILURE;
}

void rewinddir(DIR *dirp)
{
    get_orig_and_ret(rewinddir,dirp);
}

void seekdir(DIR *dirp, long loc)
{
    get_orig_and_ret(seekdir,dirp,loc);
}

long telldir(DIR *dirp)
{
    get_orig_and_ret(telldir,dirp);
    return EXIT_FAILURE;
}

int creat(const char *pathname, mode_t mode)
{
    get_orig_and_ret(creat,pathname,mode);
    return EXIT_FAILURE;
}

int open(const char *path, int oflag, ...)
{
    // Mode will be ignore in these case.
    if( !( oflag & (O_CREAT | O_TMPFILE) ) )
    {
        get_orig_and_ret(open,path,oflag);
    }else
    {
	    mode_t Mode;
        va_list List;
        va_start(List, oflag);
        Mode = va_arg(List, mode_t);
        get_orig_and_ret(open,path,oflag, Mode);
    }
    return EXIT_FAILURE;
}

int fclose(FILE *stream)
{
    get_orig_and_ret(fclose,stream);
    return EXIT_FAILURE;
}

FILE *fdopen(int fd, const char *mode)
{
    get_orig_and_ret(fdopen,fd,mode);
    return NULL;
}

int fflush(FILE *stream)
{
    get_orig_and_ret(fflush,stream);
    return EXIT_FAILURE;
}

int fgetc(FILE *stream)
{
    get_orig_and_ret(fgetc,stream);
    return EXIT_FAILURE;
}

char *fgets(char *s, int size, FILE *stream)
{
    get_orig_and_ret(fgets,s,size,stream);
    return NULL;
}

FILE *fopen(const char *path, const char *mode)
{
    get_orig_and_ret(fopen,path,mode);
    return NULL;
}

int fputs(const char *s, FILE *stream)
{
    get_orig_and_ret(fputs,s,stream);
    return EXIT_FAILURE;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    get_orig_and_ret(fread,ptr,size,nmemb,stream);
    return EXIT_FAILURE;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb,FILE *stream)
{
    get_orig_and_ret(fwrite,ptr,size,nmemb,stream);
    return EXIT_FAILURE;
}

int remove(const char *pathname)
{
    get_orig_and_ret(remove,pathname);
    return EXIT_FAILURE;
}

int rename(const char *old, const char *new)
{
    get_orig_and_ret(rename,old,new);
    return EXIT_FAILURE;
}

void setbuf(FILE *stream, char *buf)
{
    get_orig_and_ret(setbuf,stream,buf);
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    get_orig_and_ret(setvbuf,stream,buf,mode,size);
    return EXIT_FAILURE;
}

char *tempnam(const char *dir, const char *pfx)
{
    get_orig_and_ret(tempnam,dir,pfx);
    return NULL;
}

FILE *tmpfile(void)
{
    get_orig_and_ret(tmpfile);
    return NULL;
}

char *tmpnam(char *s)
{
    get_orig_and_ret(tmpnam,s);
    return NULL;
}

void *calloc(size_t nmemb, size_t size)
{
    get_orig_and_ret(calloc,nmemb,size);
    return NULL;
}

void exit(int status)
{
    get_orig_and_ret(exit,status);
}

void free(void *ptr)
{
    get_orig_and_ret(free,ptr);
}

char *getenv(const char *name)
{
    get_orig_and_ret(getenv,name);
    return NULL;
}

void *malloc(size_t size)
{
    old_malloc = dlsym(RTLD_NEXT, "malloc");
    if(old_malloc == NULL)
    {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
    return old_malloc(size);
}

char *mkdtemp(char *template)
{
    get_orig_and_ret(mkdtemp,template);
    return NULL;
}

int mkstemp(char *template)
{
    get_orig_and_ret(mkstemp,template);
    return EXIT_FAILURE;
}

int putenv(char *string)
{
    get_orig_and_ret(putenv,string);
    return EXIT_FAILURE;
}

int rand(void)
{
    get_orig_and_ret(rand);
    return EXIT_FAILURE;
}

int rand_r(unsigned int *seedp)
{
    get_orig_and_ret(rand_r,seedp);
    return EXIT_FAILURE;
}

void *realloc(void *ptr, size_t size)
{
    get_orig_and_ret(realloc,ptr,size);
    return NULL;
}

int setenv(const char *name, const char *value, int overwrite)
{
    get_orig_and_ret(setenv,name,value,overwrite);
    return EXIT_FAILURE;
}

void srand(unsigned int seed)
{
    get_orig_and_ret(srand,seed);
}

int system(const char *command)
{
    get_orig_and_ret(system,command);
    return EXIT_FAILURE;
}

int chdir(const char *path)
{
    get_orig_and_ret(chdir,path);
    return EXIT_FAILURE;
}

int chown(const char *path, uid_t owner, gid_t group)
{
    get_orig_and_ret(chown,path,owner,group);
    return EXIT_FAILURE;
}

int close(int fildes)
{
    get_orig_and_ret(close,fildes);
    return EXIT_FAILURE;
}

int dup(int fildes)
{
    get_orig_and_ret(dup,fildes);
    return EXIT_FAILURE;
}

int dup2(int fildes, int fildes2)
{
    get_orig_and_ret(dup2,fildes,fildes2);
    return EXIT_FAILURE;
}

void _exit(int status)
{
    get_orig_and_ret(_exit,status);
}

int execl(const char *path, const char *arg, ...)
{
    char *args[20];
    int i = 0;
    va_list List;
    va_start(List, arg);
    while((args[i] = va_arg(List,char *)) != NULL)
    {
        i++;
    }
    args[i] = NULL;
    get_orig_and_ret(execv, path, args);
    return EXIT_FAILURE;
}

int execle(const char *path, const char *arg, ...)
{
    char *args[20];
    char *envs[20];
    int i = 0;
    va_list List;
    va_start(List, arg);
    while((args[i] = va_arg(List,char *)) != NULL)
    {
        i++;
    }
    args[i] = NULL;
    i = 0;
    while((envs[i] = va_arg(List,char *)) != NULL)
    {
        i++;
    }
    envs[i] = NULL;
    get_orig_and_ret(execvpe, path, args, envs);
    return EXIT_FAILURE;
}

int execlp(const char *file, const char *arg, ...)
{
    char *args[20];
    int i = 0;
    va_list List;
    va_start(List, arg);
    while((args[i] = va_arg(List,char *)) != NULL)
    {
        i++;
    }
    args[i] = NULL;
    get_orig_and_ret(execvp, file, args);
    return EXIT_FAILURE;
}
int execv(const char *path, char *const argv[])
{
    get_orig_and_ret(execv,path,argv);
    return EXIT_FAILURE;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
    get_orig_and_ret(execve,path,argv,envp);
    return EXIT_FAILURE;
}

int execvp(const char *file, char *const argv[])
{
    get_orig_and_ret(execvp,file,argv);
    return EXIT_FAILURE;
}

int fchdir(int fildes)
{
    get_orig_and_ret(fchdir,fildes);
    return EXIT_FAILURE;
}

int fchown(int fildes, uid_t owner, gid_t group)
{
    get_orig_and_ret(fchown,fildes,owner,group);
    return EXIT_FAILURE;
}

pid_t fork(void)
{
    get_orig_and_ret(fork);
    return EXIT_FAILURE;
}

int fsync(int fildes)
{
    get_orig_and_ret(fsync,fildes);
    return EXIT_FAILURE;
}

int ftruncate(int fildes, off_t length)
{
    get_orig_and_ret(ftruncate,fildes,length);
    return EXIT_FAILURE;
}


char *getcwd(char *buf, size_t size)
{
    get_orig_and_ret(getcwd,buf,size);
    return NULL;
}

gid_t getegid(void)
{
    get_orig_and_ret(getegid);
    return EXIT_FAILURE;
}

uid_t geteuid(void)
{
    get_orig_and_ret(geteuid);
    return EXIT_FAILURE;
}

gid_t getgid(void)
{
    get_orig_and_ret(getgid);
    return EXIT_FAILURE;
}


int link(const char *path1, const char *path2)
{
    get_orig_and_ret(link,path1,path2);
    return EXIT_FAILURE;
}

int pipe(int fildes[2])
{
    get_orig_and_ret(pipe,fildes);
    return EXIT_FAILURE;
}

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset)
{
    get_orig_and_ret(pread,fildes,buf,nbyte,offset);
    return EXIT_FAILURE;
}

ssize_t pwrite(int fildes, const void *buf, size_t nbyte,off_t offset)
{
    get_orig_and_ret(pwrite,fildes,buf,nbyte,offset);
    return EXIT_FAILURE;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    get_orig_and_ret(read,fildes,buf,nbyte);
    return EXIT_FAILURE;
}

ssize_t readlink(const char *restrict path, char *restrict buf,size_t bufsize)
{
    get_orig_and_ret(readlink,path,buf,bufsize);
    return EXIT_FAILURE;
}

int rmdir(const char *path)
{
    get_orig_and_ret(rmdir,path);
    return EXIT_FAILURE;
}

int setegid(gid_t gid)
{
    get_orig_and_ret(setegid,gid);
    return EXIT_FAILURE;
}

int seteuid(uid_t uid)
{
    get_orig_and_ret(seteuid,uid);
    return EXIT_FAILURE;
}

int setgid(gid_t gid)
{
    get_orig_and_ret(setgid,gid);
    return EXIT_FAILURE;
}

int setuid(uid_t uid)
{
    get_orig_and_ret(setuid,uid);
    return EXIT_FAILURE;
}

unsigned int sleep(unsigned int seconds)
{
    get_orig_and_ret(sleep,seconds);
    return EXIT_FAILURE;
}

int symlink(const char *path1, const char *path2)
{
    get_orig_and_ret(symlink,path1,path2);
    return EXIT_FAILURE;
}

int unlink(const char *path)
{
    get_orig_and_ret(unlink,path);
    return EXIT_FAILURE;
}

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    get_orig_and_ret(write,fildes,buf,nbyte);
    return EXIT_FAILURE;
}
