#define _GNU_SOURCE
#include <dlfcn.h>
#include <grp.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdarg.h>
#include <pwd.h>
#include <stdlib.h>
#include <stdint.h>


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

#define monitor_MAGIC 12
static uint32_t call_count = 0;

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
static int (*old_chmod)(const char *path, mode_t mode) = NULL;
static int (*old_fchmod)(int fildes, mode_t mode) = NULL;
static int (*old_fstat)(int fildes, struct stat *buf) = NULL;
static int (*old_lstat)(const char *restrict path, struct stat *restrict buf) = NULL;
static int (*old_mkdir)(const char *path, mode_t mode) = NULL;
static int (*old_mkfifo)(const char *pathname, mode_t mode) = NULL;
static int (*old_stat)(const char *restrict path, struct stat *restrict buf) = NULL;
static mode_t (*old_umask)(mode_t cmask) = NULL;
/*More monitored api*/
static int (*old_putchar)(int in) = NULL;
static int (*old_puts)(const char *in) = NULL;
static long (*old_ftell)(FILE *in) = NULL;
static int (*old_ungetc)(int in1, FILE * in2) = NULL;
static pid_t (*old_vfork)(void) = NULL;

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

#define get_ret_old_func(orig_name,ret_type,...)  \
    ret_type RES; \
	if(get_old_name(orig_name) != NULL) { \
	    RES = get_old_name(orig_name)(__VA_ARGS__); \
	}else \
    { \
        fprintf(stderr,"Get original library call error!\n"); \
    } 

#define get_ret_old_func2(orig_name,ret_type,...)  \
    ret_type RES2; \
	if(get_old_name(orig_name) != NULL) { \
	    RES2 = get_old_name(orig_name)(__VA_ARGS__); \
	}else \
    { \
        fprintf(stderr,"Get original library call error!\n"); \
    } 

#define exec_old_func(orig_name,...)  do{ \
	if(get_old_name(orig_name) != NULL) { \
	    get_old_name(orig_name)(__VA_ARGS__); \
	}else \
    { \
        fprintf(stderr,"Get original library call error!\n"); \
    } \
}while(0)


#define GET_ORIG_RET(orig_name,ret_type,...) \
    get_old_func(orig_name); \
    get_ret_old_func(orig_name,ret_type,__VA_ARGS__) \
    call_count++;

#define GET_ORIG_RET2(orig_name,ret_type,...) \
    get_old_func(orig_name); \
    get_ret_old_func2(orig_name,ret_type,__VA_ARGS__) \
    call_count++;

#define get_orig_and_nonret(orig_name,...) do{\
    get_old_func(orig_name); \
    exec_old_func(orig_name,__VA_ARGS__); \
    call_count++; \
}while(0)

#define OUTPUT_LOC stderr
FILE *pFile = NULL;
static  __attribute__((constructor)) void Initializetion()
{    
    GET_ORIG_RET(getenv,const char *,"MONITOR_OUTPUT");
    //const char* env = getenv("MONITOR_OUTPUT");
    if((RES == NULL) || (strcmp("stderr",RES)==0))
    {
        /*do nothing*/
    }else
    {
        /*replace stderr with specific file*/
        GET_ORIG_RET2(fopen,FILE *,RES,"w");
        //pFile = fopen(RES,"w");
        pFile = RES2;
        get_orig_and_nonret(dup2,fileno(pFile),fileno(stderr));
        //dup2(fileno(pFile),fileno(stderr));
    }
}

static  __attribute__((destructor)) void Finalize()
{
    if(pFile != NULL)
    {
        fclose(pFile);
    }
}

//for FILE name
char fpath[1024];
char fname[1024];

static void get_file_path(FILE *stream)
{   
    int fd = fileno(stream); 
    /* Read out the link to our file descriptor. */
    sprintf(fpath, "/proc/self/fd/%d", fd);
    memset(fname, 0, sizeof(fname));
    get_old_func(readlink);
    old_readlink(fpath, fname, sizeof(fname)-1);
}

static void get_fd_path(int fd)
{   
    /* Read out the link to our file descriptor. */
    sprintf(fpath, "/proc/self/fd/%d", fd);
    memset(fname, 0, sizeof(fname));
    get_old_func(readlink);
    old_readlink(fpath, fname, sizeof(fname)-1);
}


uid_t getuid(void)
{
    GET_ORIG_RET(getuid,uid_t);
    fprintf(OUTPUT_LOC,"[monitor] getuid()=%s\n",getpwuid(RES)->pw_name);
    return RES;
}

int fputc(int character, FILE * stream )
{
    get_file_path(stream);
    GET_ORIG_RET(fputc,int,character,stream);
    fprintf(OUTPUT_LOC,"[monitor] fputc(%d,\"%s\") = %d\n",character,fname,RES);
    return RES;
}

struct dirent *readdir(DIR *dirp)
{
    GET_ORIG_RET(readdir,struct dirent *,dirp);
    if(RES != NULL)
    {
        fprintf(OUTPUT_LOC,"[monitor] readdir(%p)=%s\n",dirp,RES->d_name);
    }
    return RES;
}

int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result)
{
    GET_ORIG_RET(readdir_r,int,dirp,entry,result);
    if(entry != NULL)
    {
        fprintf(OUTPUT_LOC,"[monitor] readdir_r(%p, %s,%p) = %d\n",dirp,entry->d_name,result,RES);
    }
    return RES;
}

int closedir(DIR *dirp)
{
    GET_ORIG_RET(closedir,int,dirp);
    if(dirp != NULL)
    {
        fprintf(OUTPUT_LOC,"[monitor] closedir(%p)=%d\n",dirp,RES);
    }
    return RES;
}

DIR *fdopendir(int fd)
{
    GET_ORIG_RET(fdopendir,DIR *,fd);
    get_old_func(readdir);
    if(RES != NULL)
    {
        fprintf(OUTPUT_LOC,"[monitor] fdopendir(%d)=%s\n",fd,old_readdir(RES)->d_name);
    }
    return RES;
}

DIR *opendir(const char *name)
{
    GET_ORIG_RET(opendir,DIR *,name);
    fprintf(OUTPUT_LOC,"[monitor] opendir(\"%s\")=%p \n",name,RES);
    return RES;
}


void rewinddir(DIR *dirp)
{
    get_orig_and_nonret(rewinddir,dirp);
    get_old_func(readdir);
    if(dirp!=NULL)
    {
        fprintf(OUTPUT_LOC,"[monitor] rewinddir(\"%s\")\n",old_readdir(dirp)->d_name);
    }
}

void seekdir(DIR *dirp, long loc)
{
    get_orig_and_nonret(seekdir,dirp,loc);
    fprintf(OUTPUT_LOC,"[monitor] seekdir(%p,%ld)\n",dirp,loc);
}

long telldir(DIR *dirp)
{
    GET_ORIG_RET(telldir,long,dirp);
    fprintf(OUTPUT_LOC,"[monitor] telldir(%p) = %ld\n",dirp,RES);
    return RES;
}

int creat(const char *pathname, mode_t mode)
{
    GET_ORIG_RET(creat,int,pathname,mode);
    fprintf(OUTPUT_LOC,"[monitor] creat(%s,%d)=%d\n",pathname,mode,RES);
    return RES;
}

int open(const char *path, int oflag, ...)
{
    // Mode will be ignore in these case.
    if( !( oflag & (O_CREAT | O_TMPFILE) ) )
    {
        GET_ORIG_RET(open,int,path,oflag);
        fprintf(OUTPUT_LOC,"[monitor] open(%s,%d)=%d\n",path,oflag,RES);
        return RES;
    }else
    {
	    mode_t Mode;
        va_list List;
        va_start(List, oflag);
        Mode = va_arg(List, mode_t);
        GET_ORIG_RET(open,int,path,oflag,Mode);
        fprintf(OUTPUT_LOC,"[monitor] open(%s,%d,%d)=%d\n",path,oflag,Mode,RES);
        return RES;
    }
    return EXIT_FAILURE;
}

int fclose(FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fclose,int,stream);
    if(call_count > monitor_MAGIC)
    {
        fprintf(OUTPUT_LOC,"[monitor] fclose(\"%s\") = %d\n",fname,RES);
    }
    return RES;
}

FILE *fdopen(int fd, const char *mode)
{
    GET_ORIG_RET(fdopen,FILE *,fd,mode);
    get_fd_path(fd);
    fprintf(OUTPUT_LOC,"[monitor] fdopen(\"%s\",%s) = %s\n",fname,mode,fname);
    return RES;
}

int fflush(FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fflush,int,stream);
    fprintf(OUTPUT_LOC,"[monitor] fflush(\"%s\") = %d\n",fname,RES);
    return RES;
}

int fgetc(FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fgetc,int,stream);
    fprintf(OUTPUT_LOC,"[monitor] fgetc(\"%s\") = %d\n",fname,RES);
    return RES;
}

char *fgets(char *s, int size, FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fgets,char *,s,size,stream);
    fprintf(OUTPUT_LOC,"[monitor] fgets(%s,%d,\"%s\") = %s\n",s,size,fname,RES);
    return RES;
}

FILE *fopen(const char *path, const char *mode)
{
    GET_ORIG_RET(fopen,FILE *,path,mode);
    fprintf(OUTPUT_LOC,"[monitor] fopen(\"%s\",%s) = %s\n",path,mode,path);
    return RES;
}

int fputs(const char *s, FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fputs,int,s,stream);
    fprintf(OUTPUT_LOC,"[monitor] fputs(%s,\"%s\") = %d\n",s,fname,RES);
    return RES;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fread,size_t,ptr,size,nmemb,stream);
    fprintf(OUTPUT_LOC,"[monitor] fread(%p,%lu,%lu,\"%s\") = %lu\n",ptr,size,nmemb,fname,RES);
    return RES;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb,FILE *stream)
{
    get_file_path(stream);
    GET_ORIG_RET(fwrite,size_t,ptr,size,nmemb,stream);
    fprintf(OUTPUT_LOC,"[monitor] fwrite(%p,%lu,%lu,\"%s\") = %lu\n",ptr,size,nmemb,fname,RES);
    return RES;
}

int remove(const char *pathname)
{
    GET_ORIG_RET(remove,int,pathname);
    fprintf(OUTPUT_LOC,"[monitor] remove(%s) = %d\n",pathname,RES);
    return RES;
}

int rename(const char *old, const char *new)
{
    GET_ORIG_RET(rename,int,old,new);
    fprintf(OUTPUT_LOC,"[monitor] rename(%s,%s) = %d\n",old,new,RES);
    return RES;
}

void setbuf(FILE *stream, char *buf)
{
    get_orig_and_nonret(setbuf,stream,buf);
    get_file_path(stream);
    fprintf(OUTPUT_LOC,"[monitor] setbuf(\"%s\",%s)\n",fname,buf);
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size)
{
    get_file_path(stream);
    GET_ORIG_RET(setvbuf,int,stream,buf,mode,size);
    fprintf(OUTPUT_LOC,"[monitor] setvbuf(\"%s\",%s,%d,%lu) = %d\n",fname,buf,mode,size,RES);
    return RES;
}

char *tempnam(const char *dir, const char *pfx)
{
    GET_ORIG_RET(tempnam,char *,dir,pfx);
    fprintf(OUTPUT_LOC,"[monitor] tempnam(%s,%s) = %s\n",dir,pfx,RES);
    return RES;
}

FILE *tmpfile(void)
{
    GET_ORIG_RET(tmpfile,FILE *);
    get_file_path(RES);
    fprintf(OUTPUT_LOC,"[monitor] tmpfile() = %s\n",fname);
    return RES;
}

char *tmpnam(char *s)
{
    GET_ORIG_RET(tmpnam,char *,s);
    fprintf(OUTPUT_LOC,"[monitor] tmpnam(%s) = %s\n",s,RES);
    return RES;
}

void *malloc(size_t size)
{
    old_malloc = dlsym(RTLD_NEXT, "malloc");
    if(old_malloc == NULL)
    {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
    void *p = old_malloc(size);
    if(call_count > monitor_MAGIC)
    {
        fprintf(OUTPUT_LOC,"[monitor] malloc(%lu) = %p\n",size,p);
    }
    call_count++;
    return p;
}

static void* temporary_calloc(size_t x, size_t y)
{
    return NULL;
}

void *calloc(size_t nmemb, size_t size)
{
    if(!old_calloc)
    {
        //dlsym will use calloc,so use a NULL to cheat it.
        //http://blog.bigpixel.ro/2010/09/interposing-calloc-on-linux/
        old_calloc = temporary_calloc;
        old_calloc = (void *(*)(size_t, size_t)) dlsym(RTLD_NEXT, "calloc");
    }
    void *p = old_calloc(nmemb,size);
    if(call_count > monitor_MAGIC)
    {
        fprintf(OUTPUT_LOC,"[monitor] calloc(%lu,%lu) = %p\n",nmemb,size,p);
    }
    call_count++;
    return p;
}

void exit(int status)
{
    fprintf(OUTPUT_LOC,"[monitor] exit(%d)\n",status);
    get_orig_and_nonret(exit,status);
}

void free(void *ptr)
{
    get_orig_and_nonret(free,ptr);
    if(call_count > monitor_MAGIC)
    {
        fprintf(OUTPUT_LOC,"[monitor] free(%p)\n",ptr);
    }
}

char *getenv(const char *name)
{
    GET_ORIG_RET(getenv,char *,name);
    fprintf(OUTPUT_LOC,"[monitor] getenv(%s) = %s\n",name,RES);
    return RES;
}


char *mkdtemp(char *template)
{
    GET_ORIG_RET(mkdtemp,char *,template);
    fprintf(OUTPUT_LOC,"[monitor] mkdtemp(%s) = %s\n",template,RES);
    return RES;
}

int mkstemp(char *template)
{
    GET_ORIG_RET(mkstemp,int,template);
    fprintf(OUTPUT_LOC,"[monitor] mkstemp(%s) = %d\n",template,RES);
    return RES;
}

int putenv(char *string)
{
    GET_ORIG_RET(putenv,int,string);
    fprintf(OUTPUT_LOC,"[monitor] putenv(%s) = %d\n",string,RES);
    return RES;
}

int rand(void)
{
    GET_ORIG_RET(rand,int);
    fprintf(OUTPUT_LOC,"[monitor] rand() = %d\n",RES);
    return RES;
}

int rand_r(unsigned int *seedp)
{
    GET_ORIG_RET(rand_r,int,seedp);
    fprintf(OUTPUT_LOC,"[monitor] rand_r(%p) = %d\n",seedp,RES);
    return RES;
}

void *realloc(void *ptr, size_t size)
{
    GET_ORIG_RET(realloc,void *,ptr,size);
    fprintf(OUTPUT_LOC,"[monitor] realloc(%p,%lu) = %p\n",ptr,size,RES);
    return RES;
}

int setenv(const char *name, const char *value, int overwrite)
{
    GET_ORIG_RET(setenv,int,name,value,overwrite);
    fprintf(OUTPUT_LOC,"[monitor] setenv(%s,%s,%d) = %d\n",name,value,overwrite,RES);
    return RES;
}

void srand(unsigned int seed)
{
    get_orig_and_nonret(srand,seed);
    fprintf(OUTPUT_LOC,"[monitor] srand(%d)\n",seed);
}

int system(const char *command)
{
    GET_ORIG_RET(system,int,command);
    fprintf(OUTPUT_LOC,"[monitor] system(%s) = %d\n",command,RES);
    return RES;
}

int chdir(const char *path)
{
    GET_ORIG_RET(chdir,int,path);
    fprintf(OUTPUT_LOC,"[monitor] chdir(%s) = %d\n",path,RES);
    return RES;
}

int chown(const char *path, uid_t owner, gid_t group)
{
    GET_ORIG_RET(chown,int,path,owner,group);
    fprintf(OUTPUT_LOC,"[monitor] chown(%s,%d,%d) = %d\n",path,owner,group,RES);
    return RES;
}

int close(int fildes)
{
    GET_ORIG_RET(close,int,fildes);
    fprintf(OUTPUT_LOC,"[monitor] close(%d) = %d\n",fildes,RES);
    return RES;
}

int dup(int fildes)
{
    GET_ORIG_RET(dup,int,fildes);
    fprintf(OUTPUT_LOC,"[monitor] dup(%d) = %d\n",fildes,RES);
    return RES;
}

int dup2(int fildes, int fildes2)
{
    GET_ORIG_RET(dup2,int,fildes,fildes2);
    fprintf(OUTPUT_LOC,"[monitor] dup2(%d,%d) = %d\n",fildes,fildes2,RES);
    return RES;
}

void _exit(int status)
{
    fprintf(OUTPUT_LOC,"[monitor] _exit(%d)\n",status);
    get_orig_and_nonret(_exit,status);
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
    GET_ORIG_RET(execv,int,path,args);
    fprintf(OUTPUT_LOC,"[monitor] execl(%s,%s,...) = %d\n",path,arg,RES);
    return RES;
}

int execle(const char *path, const char *arg, ...)
{
    char *args[20];
    char **envs;
    int i = 0;
    va_list List;
    va_start(List, arg);
    while((args[i] = va_arg(List,char *)) != NULL)
    {
        i++;
    }
    args[i] = NULL;
    envs = va_arg(List,char **);
    GET_ORIG_RET(execvpe,int,path,args,envs);
    fprintf(OUTPUT_LOC,"[monitor] execle(%s,%s,%s...) = %d\n",path,arg,envs[0],RES);
    return RES;
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
    GET_ORIG_RET(execvp,int,file,args);
    fprintf(OUTPUT_LOC,"[monitor] execlp(%s,%s,...) = %d\n",file,arg,RES);
    return RES;
}

int execv(const char *path, char *const argv[])
{
    GET_ORIG_RET(execv,int,path,argv);
    fprintf(OUTPUT_LOC,"[monitor] execv(%s,%s...) = %d\n",path,argv[0],RES);
    return RES;
}

int execve(const char *path, char *const argv[], char *const envp[])
{
    GET_ORIG_RET(execve,int,path,argv,envp);
    fprintf(OUTPUT_LOC,"[monitor] execve(%s,%s...,%s...) = %d\n",path,argv[0],envp[0],RES);
    return RES;
}

int execvp(const char *file, char *const argv[])
{
    GET_ORIG_RET(execvp,int,file,argv);
    fprintf(OUTPUT_LOC,"[monitor] execvp(%s,%s...) = %d\n",file,argv[0],RES);
    return RES;
}

int fchdir(int fildes)
{
    GET_ORIG_RET(fchdir,int,fildes);
    fprintf(OUTPUT_LOC,"[monitor] fchdir(%d) = %d\n",fildes,RES);
    return RES;
}

int fchown(int fildes, uid_t owner, gid_t group)
{
    GET_ORIG_RET(fchown,int,fildes,owner,group);
    fprintf(OUTPUT_LOC,"[monitor] fchown(%d,%d,%d) = %d\n",fildes,owner,group,RES);
    return RES;
}

pid_t fork(void)
{
    GET_ORIG_RET(fork,pid_t);
    fprintf(OUTPUT_LOC,"[monitor] fork() = %d\n",RES);
    return RES;
}

int fsync(int fildes)
{
    GET_ORIG_RET(fork,pid_t);
    fprintf(OUTPUT_LOC,"[monitor] fsync(%d) = %d\n",fildes,RES);
    return RES;
}

int ftruncate(int fildes, off_t length)
{
    GET_ORIG_RET(ftruncate,int,fildes,length);
    fprintf(OUTPUT_LOC,"[monitor] ftruncate(%d,%ld) = %d\n",fildes,length,RES);
    return RES;
}


char *getcwd(char *buf, size_t size)
{
    GET_ORIG_RET(getcwd,char *,buf,size);
    fprintf(OUTPUT_LOC,"[monitor] getcwd(%s,%lu) = %s\n",buf,size,RES);
    return RES;
}

gid_t getegid(void)
{
    GET_ORIG_RET(getegid,gid_t);
    fprintf(OUTPUT_LOC,"[monitor] getegid() = %s\n",getgrgid(RES)->gr_name);
    return RES;
}

uid_t geteuid(void)
{
    GET_ORIG_RET(geteuid,uid_t);
    fprintf(OUTPUT_LOC,"[monitor] geteuid() = %s\n",getpwuid(RES)->pw_name);
    return RES;
}

gid_t getgid(void)
{
    GET_ORIG_RET(getgid,gid_t);
    fprintf(OUTPUT_LOC,"[monitor] getgid() = %s\n",getgrgid(RES)->gr_name);
    return RES;
}

int link(const char *path1, const char *path2)
{
    GET_ORIG_RET(link,int,path1,path2);
    fprintf(OUTPUT_LOC,"[monitor] link(%s,%s) = %d\n",path1,path2,RES);
    return RES;
}

int pipe(int fildes[2])
{
    GET_ORIG_RET(pipe,int,fildes);
    fprintf(OUTPUT_LOC,"[monitor] pipe(%p) = %d\n",fildes,RES);
    return RES;
}

ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset)
{
    GET_ORIG_RET(pread,int,fildes,buf,nbyte,offset);
    fprintf(OUTPUT_LOC,"[monitor] pread(%d,%p,%lu,%lu) = %d\n",fildes,buf,nbyte,offset,RES);
    return RES;
}

ssize_t pwrite(int fildes, const void *buf, size_t nbyte,off_t offset)
{
    GET_ORIG_RET(pwrite,ssize_t,fildes,buf,nbyte,offset);
    fprintf(OUTPUT_LOC,"[monitor] pwrite(%d,%p,%lu,%lu) = %ld\n",fildes,buf,nbyte,offset,RES);
    return RES;
}

ssize_t read(int fildes, void *buf, size_t nbyte)
{
    GET_ORIG_RET(read,ssize_t,fildes,buf,nbyte);
    fprintf(OUTPUT_LOC,"[monitor] read(%d,%p,%lu) = %ld\n",fildes,buf,nbyte,RES);
    return RES;
}

ssize_t readlink(const char *restrict path, char *restrict buf,size_t bufsize)
{
    GET_ORIG_RET(readlink,ssize_t,path,buf,bufsize);
    fprintf(OUTPUT_LOC,"[monitor] readlink(%s,%s,%lu) = %ld\n",path,buf,bufsize,RES);
    return RES;
}

int rmdir(const char *path)
{
    GET_ORIG_RET(rmdir,int,path);
    fprintf(OUTPUT_LOC,"[monitor] rmdir(%s) = %d\n",path,RES);
    return RES;
}

int setegid(gid_t gid)
{
    GET_ORIG_RET(setegid,int,gid);
    fprintf(OUTPUT_LOC,"[monitor] setegid(%s) = %d\n",getgrgid(RES)->gr_name,RES);
    return RES;
}

int seteuid(uid_t uid)
{
    GET_ORIG_RET(seteuid,int,uid);
    fprintf(OUTPUT_LOC,"[monitor] seteuid(%s) = %d\n",getpwuid(uid)->pw_name,RES);
    return RES;
}

int setgid(gid_t gid)
{
    GET_ORIG_RET(setegid,int,gid);
    fprintf(OUTPUT_LOC,"[monitor] setgid(%s) = %d\n",getgrgid(RES)->gr_name,RES);
    return RES;
}

int setuid(uid_t uid)
{
    GET_ORIG_RET(setuid,int,uid);
    fprintf(OUTPUT_LOC,"[monitor] setuid(%s) = %d\n",getpwuid(uid)->pw_name,RES);
    return RES;
}

unsigned int sleep(unsigned int seconds)
{
    GET_ORIG_RET(sleep,unsigned int,seconds);
    fprintf(OUTPUT_LOC,"[monitor] sleep(%d) = %d\n",seconds,RES);
    return RES;
}

int symlink(const char *path1, const char *path2)
{
    GET_ORIG_RET(symlink,int,path1,path2);
    fprintf(OUTPUT_LOC,"[monitor] symlink(%s,%s) = %d\n",path1,path2,RES);
    return RES;
}

int unlink(const char *path)
{
    GET_ORIG_RET(unlink,int,path);
    fprintf(OUTPUT_LOC,"[monitor] unlink(%s) = %d\n",path,RES);
    return RES;
}

ssize_t write(int fildes, const void *buf, size_t nbyte)
{
    GET_ORIG_RET(write,int,fildes,buf,nbyte);
    fprintf(OUTPUT_LOC,"[monitor] write(%d,%p,%lu) = %d\n",fildes,buf,nbyte,RES);
    return RES;
}

int chmod(const char *path, mode_t mode)
{
    GET_ORIG_RET(chmod,int,path,mode);
    fprintf(OUTPUT_LOC,"[monitor] chmod(%s,%d) = %d\n",path,mode,RES);
    return RES;
}

int fchmod(int fildes, mode_t mode)
{
    GET_ORIG_RET(fchmod,int,fildes,mode);
    fprintf(OUTPUT_LOC,"[monitor] fchmod(%d,%d) = %d\n",fildes,mode,RES);
    return RES;
}

int fstat(int fildes, struct stat *buf)
{
    GET_ORIG_RET(fstat,int,fildes,buf);
    fprintf(OUTPUT_LOC,"[monitor] fstat(%d,type and permission(st_mode)=0x%.8X, size=%d bytes) = %d\n",fildes,buf->st_mode,(int)buf->st_size,RES);
    return RES;
}

int lstat(const char *restrict path, struct stat *restrict buf)
{
    GET_ORIG_RET(lstat,int,path,buf);
    fprintf(OUTPUT_LOC,"[monitor] lstat(%s,type and permission(st_mode)=0x%.8X, size=%d bytes) = %d\n",path,buf->st_mode,(int)buf->st_size,RES);
    return RES;
}

int mkdir(const char *path, mode_t mode)
{
    GET_ORIG_RET(mkdir,int,path,mode);
    fprintf(OUTPUT_LOC,"[monitor] mkdir(%s,%d) = %d\n",path,mode,RES);
    return RES;
}

int mkfifo(const char *pathname, mode_t mode)
{
    GET_ORIG_RET(mkfifo,int,pathname,mode);
    fprintf(OUTPUT_LOC,"[monitor] mkfifo(%s,%d) = %d\n",pathname,mode,RES);
    return RES;
}

int stat(const char *restrict path, struct stat *restrict buf)
{
    GET_ORIG_RET(stat,int,path,buf);
    fprintf(OUTPUT_LOC,"[monitor] stat(%s,type and permission(st_mode)=0x%.8X, size=%d bytes) = %d\n",path,buf->st_mode,(int)buf->st_size,RES);
    return RES;
}

static int (*old___lxstat)(int ver, const char * path, struct stat * stat_buf);
int __lxstat(int ver, const char * path, struct stat * stat_buf)
{
    GET_ORIG_RET(__lxstat,int,ver,path,stat_buf);
    fprintf(OUTPUT_LOC,"[monitor] __lxstat(%d,%s,type and permission(st_mode)=0x%.8X, size=%d bytes) = %d\n",ver,path,stat_buf->st_mode,(int)stat_buf->st_size,RES);
    return RES;
}

mode_t umask(mode_t cmask)
{
    GET_ORIG_RET(umask,mode_t,cmask);
    fprintf(OUTPUT_LOC,"[monitor] umask(%d) = %d\n",cmask,RES);
    return RES;
}
/*More monitored api*/
int putchar(int in)
{
    GET_ORIG_RET(putchar,int,in);
    fprintf(OUTPUT_LOC,"[monitor] putchar(%d) = %d\n",in,RES);
    return RES;
}

int puts(const char *in)
{
    GET_ORIG_RET(puts,int,in);
    fprintf(OUTPUT_LOC,"[monitor] puts(%s) = %d\n",in,RES);
    return RES;
}

long ftell(FILE *in)
{
    GET_ORIG_RET(ftell,long,in);
    get_file_path(in);
    fprintf(OUTPUT_LOC,"[monitor] ftell(%s) = %ld\n",fname,RES);
    return RES;
}

int ungetc(int in1, FILE *in2)
{
    GET_ORIG_RET(ungetc,int,in1,in2);
    get_file_path(in2);
    fprintf(OUTPUT_LOC,"[monitor] ungetc(%d,%s) = %d\n",in1,fname,RES);
    return RES;
}

pid_t vfork(void)
{
    GET_ORIG_RET(vfork,int);
    fprintf(OUTPUT_LOC,"[monitor] vfork() = %d\n",RES);
    return RES;
}
