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
static int (*old_chmod)(const char *path, mode_t mode) = NULL;
static int (*old_fchmod)(int fildes, mode_t mode) = NULL;
static int (*old_fstat)(int fildes, struct stat *buf) = NULL;
static int (*old_lstat)(const char *restrict path, struct stat *restrict buf) = NULL;
static int (*old_mkdir)(const char *path, mode_t mode) = NULL;
static int (*old_mkfifo)(const char *pathname, mode_t mode) = NULL;
static int (*old_stat)(const char *restrict path, struct stat *restrict buf) = NULL;
static mode_t (*old_umask)(mode_t cmask) = NULL;

static  __attribute__((constructor)) void Initializetion()
{    
    printf("Hello Injection\n");
#define OUTPUT_LOC stderr
}

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

#define get_ret_old_func(orig_name,ret_type,...)  \
    ret_type RES; \
	if(get_old_name(orig_name) != NULL) { \
	    RES = get_old_name(orig_name)(__VA_ARGS__); \
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

#define get_orig_and_ret(orig_name,...) do{\
    get_old_func(orig_name); \
    return_old_func(orig_name,__VA_ARGS__); \
}while(0)

#define GET_ORIG_RET(orig_name,ret_type,...) \
    get_old_func(orig_name); \
    get_ret_old_func(orig_name,ret_type,__VA_ARGS__) \

#define get_orig_and_nonret(orig_name,...) do{\
    get_old_func(orig_name); \
    exec_old_func(orig_name,__VA_ARGS__); \
}while(0)

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
    get_orig_and_ret(fputc,character,stream);
    return EXIT_FAILURE;
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
    get_file_path(stream);
    GET_ORIG_RET(fclose,int,stream);
    fprintf(OUTPUT_LOC,"[monitor] fclose(\"%s\") = %d\n",fname,RES);
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
    get_orig_and_ret(tempnam,dir,pfx);
    return NULL;
}

FILE *tmpfile(void)
{
    GET_ORIG_RET(tmpfile,FILE *);
    get_file_path(RES);
    fprintf(OUTPUT_LOC,"[monitor] tmpfile() = \n",fname);
    return RES;
}

char *tmpnam(char *s)
{
    get_orig_and_ret(tmpnam,s);
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
    return old_calloc(nmemb,size);
}

void exit(int status)
{
    get_orig_and_nonret(exit,status);
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
    get_orig_and_ret(execv, path, args);
    return EXIT_FAILURE;
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
    GET_ORIG_RET(setegid,int,gid);
    fprintf(OUTPUT_LOC,"[monitor] setegid(%s) = %d\n",getgrgid(RES)->gr_name,RES);
    return RES;
}

int seteuid(uid_t uid)
{
    GET_ORIG_RET(seteuid,int,uid);
    fprintf(OUTPUT_LOC,"[monitor] seteuid(%s)\n",getpwuid(uid)->pw_name,RES);
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

int chmod(const char *path, mode_t mode)
{
    get_orig_and_ret(chmod,path,mode);
    return EXIT_FAILURE;
}

int fchmod(int fildes, mode_t mode)
{
    get_orig_and_ret(fchmod,fildes,mode);
    return EXIT_FAILURE;
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
    get_orig_and_ret(mkdir,path,mode);
    return EXIT_FAILURE;
}

int mkfifo(const char *pathname, mode_t mode)
{
    get_orig_and_ret(mkfifo,pathname,mode);
    return EXIT_FAILURE;
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
    get_orig_and_ret(umask,cmask);
    return EXIT_FAILURE;
}
