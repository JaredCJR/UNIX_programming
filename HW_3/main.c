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

typedef struct{
    int in;
    int out;
}PIPE_MAP;

#define PIPE_IN_FD   0
#define PIPE_OUT_FD  1

int REDIRECT_IN = 0;
int REDIRECT_OUT = 0;
char filename_in[1024];
char filename_out[1024];
pid_t pid_array[MAX_PIPE_NUM] = {-1};

pid_t shell_pid;
int parse_end = 0;
volatile int core_dump = 0;

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
static int parser(char *cmd,int *argc,char *argv[MAX_ARGC], PIPE_MAP *pipe_map,int is_pipe_in)
{
    const char* deli = " \n\r";
    argv[*argc = 0] = strtok(cmd, deli);
    (*argc)++;
    if(argv[0] == NULL)
    {
        return 0;
    }
    if(is_pipe_in)
    {
        pipe_map->in = 1;
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
            pipe_map->out = 1;
            parse_end = 0;
            return offset;
        }
        (*argc)++;
    }
    return 0;
}

static void sub_command(int fd_in,int fd_out,char *argv_store[MAX_ARGC],int is_last)
{
    pid_t pid;
    if ((pid = fork ()) == 0)
    {
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
    }
}


static void run(char *cmd)
{
    REDIRECT_IN = 0;
    REDIRECT_OUT = 0;
    int argc_store[MAX_PIPE_NUM];
    char *argv_store[MAX_PIPE_NUM][MAX_ARGC];
    PIPE_MAP pipes_map[MAX_PIPE_NUM] = {{0,0}};
    int cmd_idx = 0;
    int cmd_offset = 0;
    int is_pipe_in = 0;
    parse_end = 0;
    while(cmd_offset += parser(cmd+cmd_offset, &argc_store[cmd_idx], argv_store[cmd_idx],&pipes_map[cmd_idx],is_pipe_in))
    {
        /*Managing multiple commands relative variable*/
        if(argc_store[cmd_idx] == 0 || parse_end)
        {
            break;
        }
        parse_end = 1;
        is_pipe_in = pipes_map[cmd_idx].out ? 1 : 0;
        cmd_idx++;
    }
    /*fork pipeline*/
    int pipe_fd[2];
    int in = STDIN_FILENO;
    for(int i = 0;i < cmd_idx;i++)//the last command should output to the original STDOUT_FILENO
    {
        pipe(pipe_fd);
        sub_command(in,pipe_fd[PIPE_OUT_FD],argv_store[i],cmd_idx == i);
        close(pipe_fd[PIPE_OUT_FD]);
        in = pipe_fd[PIPE_IN_FD];
    }
    //last command
    int pid;
    int fd_in;
    int fd_out;
    if ((pid = fork ()) == 0)
    {
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
        if(execvp(argv_store[cmd_idx][0], argv_store) == -1)
        {
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[cmd_idx][0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
    }
    int status;
    while(waitpid(pid, &status, WNOHANG) <= 0)
    {
    }
    //parent
    //TODO:setup foreground/background pg
    //TODO:create pipe
    //setpgid(pid,gid);
    
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


    size_t buf_len = 1024;
    char *cmd_buf = (char*)malloc(buf_len);
    if( cmd_buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
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
