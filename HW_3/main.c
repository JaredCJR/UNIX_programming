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
#define MAX_PIPE_NUM   10

#define PIPE_IN_FD   0
#define PIPE_OUT_FD  1

#define MAX_PG     100

int REDIRECT_IN = 0;
int REDIRECT_OUT = 0;
char filename_in[1024];
char filename_out[1024];
pid_t pid_array[MAX_PIPE_NUM] = {-1};
int BACKGROUND_MODE = 0;

pid_t shell_pgid;
pid_t shell_pid;
int parse_end = 0;
volatile int core_dump = 0;
pid_t pgid_map[MAX_PG] = {0};
int PGID_MAP_IDX = 0;

int orig_sigint_handler;
int orig_sigquit_handler;

void sig_handler(int signo)
{
    if (signo == SIGINT)//looks for ctrl-c which has a value of 2
    {
        if(getpid() != shell_pid)
        {
            signal(SIGINT,orig_sigint_handler);
            raise(signo);
        }
    }
    else if (signo == SIGQUIT)//looks for ctrl-\ which has a value of 9
    {
        if(getpid() != shell_pid)
        {
            signal(SIGQUIT,orig_sigquit_handler);
            core_dump = 1;
            raise(signo);
        }
    }else if(signo == SIGCHLD)
    {
        /*
        if((getpid() == shell_pid) && core_dump)
        {
            core_dump = 0;
            printf("Quit (core dumped)\n");
        }
        */
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

/*
 * return value:
 * 0:non-pipe_mode
 * others: cmd offset for pipe mode
 */
static int parser(char *cmd,int *argc,char *argv[MAX_ARGC])
{
    const char* deli = " \n\r";
    argv[*argc = 0] = strtok(cmd, deli);
    (*argc)++;
    if(argv[0] == NULL)
    {
        return 0;
    }
    while ((argv[*argc] = strtok(NULL, deli)) != NULL)
    {
        if(strcmp(argv[*argc],"<") == 0)
        {
            argv[*argc] = NULL;//clear "<"
            redirect_input(strtok(NULL, deli));
            return 0;
        }
        if(strcmp(argv[*argc],">") == 0)
        {
            argv[*argc] = NULL;//clear ">"
            redirect_output(strtok(NULL, deli));
            return 0;
        }
        if(strcmp(argv[*argc],"|") == 0)
        {
            int offset = argv[*argc] - cmd + 2;//addition 1 for strtok()'s '\0'
            argv[*argc] = NULL;//clear "|"
            parse_end = 0;
            return offset;
        }
        if(strcmp(argv[*argc],"&") == 0)
        {
            argv[*argc] = NULL;//clear ">"
            BACKGROUND_MODE = 1;
            return 0;
        }
        (*argc)++;
    }
    return 0;
}

static void sub_command(int fd_in,int fd_out,char *argv_store[MAX_ARGC], int is_first,pid_t pgid_idx)
{
    pid_t pid;
    if ((pid = fork ()) == 0)
    {
        if(is_first)
        {
            if(setpgid(0,0))
            {
                perror("setpgid failed\n");
            }
            pgid_map[pgid_idx] = getpgid(0);
        }else
        {
            if(setpgid(0,pgid_map[pgid_idx]))
            {
                perror("setpgid failed\n");
            }
        }
        if(fd_in != STDIN_FILENO)
        {
            dup2(fd_in,STDIN_FILENO);
            close(fd_in);
        }
        if(fd_out != STDOUT_FILENO)
        {
            dup2(fd_out,STDOUT_FILENO);
            close(fd_out);
        }
        if(execvp(argv_store[0], argv_store) == -1)
        {
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
    }else if(pid > 0 && is_first)
    {
       pgid_map[pgid_idx] = pid; 
    }
}


static void run(char *cmd)
{
    REDIRECT_IN = 0;
    REDIRECT_OUT = 0;
    int argc_store[MAX_PIPE_NUM];
    char *argv_store[MAX_PIPE_NUM][MAX_ARGC];
    int cmd_idx = 0;
    int cmd_offset = 0;
    parse_end = 0;
    while(cmd_offset += parser(cmd+cmd_offset, &argc_store[cmd_idx], argv_store[cmd_idx]))
    {
        /*Managing multiple commands relative variable*/
        if(argc_store[cmd_idx] == 0 || parse_end)
        {
            break;
        }
        parse_end = 1;
        cmd_idx++;
    }
    /*fork pipeline*/
    int pipe_fd[2];
    int in = STDIN_FILENO;
    int pgid_save = PGID_MAP_IDX;
    for(int i = 0;i < cmd_idx;i++)//the last command should output to the original STDOUT_FILENO
    {
        pipe(pipe_fd);
        sub_command(in,pipe_fd[PIPE_OUT_FD],argv_store[i],i==0,pgid_save);
        close(pipe_fd[PIPE_OUT_FD]);
        in = pipe_fd[PIPE_IN_FD];
    }
    //last command
    int pid;
    int fd_in;
    int fd_out;
    if ((pid = fork ()) == 0)
    {
        if(cmd_idx == 0)
        {
            if(setpgid(0,0))
            {
                perror("setpgid failed\n");
            }
            pgid_map[PGID_MAP_IDX] = getpgid(0);
        }else
        {
            if(setpgid(0,pgid_map[PGID_MAP_IDX]))
            {
                perror("setpgid failed\n");
            }
        }
        if(in != STDIN_FILENO)
        {
            dup2(in, STDIN_FILENO);
            close(in);
        }
        if(REDIRECT_IN)
        {
            fd_in = open(filename_in, O_RDONLY);
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }
        if(REDIRECT_OUT)
        {
            fd_out = open(filename_out, O_WRONLY | O_CREAT, 0666);
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }
        if(execvp(argv_store[cmd_idx][0], argv_store[cmd_idx]) == -1)
        {
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[cmd_idx][0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
    }else
    {
        if((pid > 0) && (cmd_idx == 0))
        {
            pgid_map[PGID_MAP_IDX] = pid;
        }
    }
    //TODO:setup foreground/background pg
	if( tcsetpgrp(STDIN_FILENO, pgid_map[PGID_MAP_IDX]) == -1 ||
	    tcsetpgrp(STDOUT_FILENO, pgid_map[PGID_MAP_IDX]) == -1 ||
	    tcsetpgrp(STDERR_FILENO, pgid_map[PGID_MAP_IDX]) == -1 )
	{
		perror("Failed to set foreground process for command\n");
    }
    PGID_MAP_IDX++;
    int status;
    while(waitpid(pid, &status, WNOHANG) <= 0)
    {
    }
/*    
	if( tcsetpgrp(STDIN_FILENO, shell_pgid) == -1 ||
	    tcsetpgrp(STDOUT_FILENO, shell_pgid) == -1 ||
	    tcsetpgrp(STDERR_FILENO, shell_pgid) == -1 )
	{
		perror("Failed to set foreground process for command\n");
    }
*/  
}

int main(void)
{
	/*Catch SIGNAL*/
    if ((orig_sigint_handler = signal(SIGINT, sig_handler)) == SIG_ERR)
        printf("\ncan't catch SIGINT\n");
    if ((orig_sigquit_handler = signal(SIGQUIT, sig_handler)) == SIG_ERR)
        printf("\ncan't catch SIGQUIT\n");
    if (signal(SIGCHLD, sig_handler) == SIG_ERR)
        printf("\ncan't catch SIGCHLD\n");

    PGID_MAP_IDX = 0;
    size_t buf_len = 1024;
    char *cmd_buf = (char*)malloc(buf_len);
    if( cmd_buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    shell_pgid = getpgid(0);
    shell_pid = getpid();
    while(1)
    {
        printf("shell-prompt$ ");
		fflush(NULL);
        getline(&cmd_buf, &buf_len, stdin);
        run(cmd_buf);
    }
    free(cmd_buf);
}
