#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_ARGC       64

int REDIRECT_IN = 0;
int REDIRECT_OUT = 0;
char filename_in[1024];
char filename_out[1024];

void sig_handler(int signo)
{
    if (signo == SIGINT)//looks for ctrl-c which has a value of 2
    {
    }
    else if (signo == SIGQUIT)//looks for ctrl-\ which has a value of 9
    {
    }
}

static void redirect_input(char *fname)
{
    strcpy(filename_in,fname);
    REDIRECT_IN = 1;
}

static void redirect_output(char *fname)
{
    strcpy(filename_out,fname);
    REDIRECT_OUT = 1;
}

static void split(char *cmd,int *argc,char *argv[MAX_ARGC])
{
    const char* deli = " \n";
    argv[*argc = 0] = strtok(cmd, deli);
    if(argv[0] == NULL)
    {
        return;
    }
    while ((argv[++(*argc)] = strtok(NULL, deli)) != NULL)
    {
        if(strcmp(argv[*argc],"<") == 0)
        {
            argv[(*argc)--] = NULL;//clear "<"
            redirect_input(strtok(NULL, deli));
        }
        if(strcmp(argv[*argc],">") == 0)
        {
            argv[(*argc)--] = NULL;//clear ">"
            redirect_output(strtok(NULL, deli));
        }
    }
}

static void run(char *cmd)
{
    REDIRECT_IN = 0;
    REDIRECT_OUT = 0;
    int argc_store;
    char *argv_store[MAX_ARGC];
    split(cmd, &argc_store, argv_store);
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
        int fd_in;
        int fd_out;
        if(REDIRECT_IN)
        {
            fd_in = open(filename_in, O_RDONLY);
            dup2(fd_in, STDIN_FILENO);
        }
        if(REDIRECT_OUT)
        {
            fd_out = open(filename_out, O_WRONLY | O_CREAT);
            dup2(fd_out, STDOUT_FILENO);
        }

        if(execvp(argv_store[0], argv_store) == -1)
        {
            /*
            for(int i = 0;i < argc_store;i++)
            {
                printf("%d:%s\n",i,argv_store[i]);
            }
            */
            if(REDIRECT_IN)
            {
                close(fd_in);
            }
            if(REDIRECT_OUT)
            {
                close(fd_out);
            }
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
        if(REDIRECT_IN)
        {
            close(fd_in);
        }
        if(REDIRECT_OUT)
        {
            close(fd_out);
        }
        REDIRECT_IN = 0;
        REDIRECT_OUT = 0;
    }
    int status;
    waitpid(pid, &status, 0);//parent
}

int main(void)
{
	/*Catch SIGNAL*/
    if (signal(SIGINT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    if (signal(SIGQUIT, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGQUIT\n");


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
