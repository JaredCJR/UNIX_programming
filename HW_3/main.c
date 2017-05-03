#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<errno.h>

#define MAX_ARGC       64
#define MAX_ARGLEN     128

static void split(char *cmd,int *argc,char *argv[MAX_ARGC])
{
    const char* deli = " \n";
    argv[*argc = 0] = strtok(cmd, deli);
    if(argv[0] == NULL)
    {
        return;
    }
    while ((argv[++(*argc)] = strtok(NULL, deli)) != NULL)
        ;
}

static void run(char *cmd)
{
    int argc_store;
    char *argv_store[MAX_ARGC];
    split(cmd, &argc_store, &argv_store);
    /*
    for(int i = 0;i < argc_store;i++)
    {
        printf("%d:%s\n",i,argv_store[i]);
    }
    */
    if(argc_store == 0)
    {
        return;
    }
    int pid = -1;
    if((pid = fork()) == 0)//child
    {
        if(execvp(argv_store[0], argv_store) == -1)
        {
            for(int i = 0;i < argc_store;i++)
            {
                printf("%d:%s\n",i,argv_store[i]);
            }
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
    }
    int status;
    waitpid(pid, &status, 0);//parent
}

int main(void)
{
    size_t buf_len = 1024;
    char *cmd_buf = (char*)malloc(buf_len);
    if( cmd_buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    while(1)
    {
        printf("shell-prompt$ ");
		fflush(NULL);
        getline(&cmd_buf, &buf_len, stdin);
        run(cmd_buf);
    }
    free(cmd_buf);
}
