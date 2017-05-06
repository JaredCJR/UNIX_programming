#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "glob.h"
#include <string>
#include <stdint.h>
#include <vector>
#include <iostream>


#define MAX_ARGC              256
#define MAX_PIPE_NUM           20

#define PIPE_IN_FD              0
#define PIPE_OUT_FD             1

#define MAX_PG                200

#define MAGIC_BUILDIN     -109487
#define MAGIC_INVALID_CMD -999487

int REDIRECT_IN = 0;
int REDIRECT_OUT = 0;
char filename_in[1024];
char filename_out[1024];
pid_t pid_array[MAX_PIPE_NUM] = {-1};
int BACKGROUND_MODE = 0;
char cwd[1024];
int WaitForChildren = 1;

int parse_end = 0;
volatile int core_dump = 0;
sigset_t SignalSet;
glob_t glob_result;

std::vector<pid_t> PGs_table;
uint32_t curr_PG;
std::vector<std::string> PGs_cmd;
#define SHELL_PG_IDX    0

void KillChildren(int Signal)
{
    if(getpid() != PGs_table.front())
    {
	    if( kill(-PGs_table.back(), Signal) == -1 )
        {
		    perror("Failed to kill children");
        }
        PGs_table.pop_back();
        curr_PG--;
    }
}

void SuspendChildren(int Signal)
{
    if(getpid() == PGs_table.front())//shell
    {
    }else
    {
        if(signal(SIGTSTP,SIG_DFL) == SIG_ERR)
        {
            perror("Ctrl+z in child will not works");
        }
        raise(SIGSTOP);
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
        char *match;
        //expanding metacharachers
		if(((match = strchr(argv[*argc],'\?')) != NULL) ||
		   ((match = strchr(argv[*argc],'*')) != NULL)  )
        {
            match = '\0';
            char cwd_copy[1024];
            strcpy(cwd_copy,cwd);
            strcat(cwd_copy,"/");
            strcat(cwd_copy,argv[*argc]);
            glob(cwd_copy,GLOB_TILDE,NULL,&glob_result);
            for(unsigned int i=0;i<glob_result.gl_pathc;i++)
            {
                argv[(*argc)++] = glob_result.gl_pathv[i];
            }
            continue;
        }
        if(strcmp(argv[*argc],"<") == 0)
        {
            argv[*argc] = NULL;//clear "<"
            redirect_input(strtok(NULL, deli));
            (*argc)--;
        }
        if(strcmp(argv[*argc],">") == 0)
        {
            argv[*argc] = NULL;//clear ">"
            redirect_output(strtok(NULL, deli));
            (*argc)--;
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

static void do_environVar(char *cmd,char *arg)
{
    char *var_name = strtok(arg,"=\n\r");
    if(var_name == NULL)
    {
        fprintf(stderr,"Variable not specified!\n");
        return;
    }
    if(strcmp(cmd,"export") == 0)
    {
        //Usage: export foo=asdfgasd
        int overwrite = 1;
        char *var_value = strtok(NULL,"=");
        setenv(var_name, var_value, overwrite);
    }else//unset
    {
        //Usage: unset VAR_NAME
        unsetenv(var_name);
    }
}

static pid_t sub_command(int fd_in,int fd_out,char *argv_store[MAX_ARGC], int is_first)
{
    if((strcmp(argv_store[0],"export") == 0)||(strcmp(argv_store[0],"unset") == 0))
    {
        do_environVar(argv_store[0], argv_store[1]);
        return MAGIC_BUILDIN;
    }
    if((strcmp(argv_store[0],"jobs") == 0))
    {
        for(uint32_t i = SHELL_PG_IDX+1;i < PGs_table.size();i++)
        {
            printf("[%d]  Stopped  pgid=%d       %s",i,PGs_table[i],PGs_cmd[i].c_str());
        }
        return MAGIC_BUILDIN;
    }
    if((strcmp(argv_store[0],"fg") == 0))
    {
        int status;
        kill(-PGs_table.back(),SIGCONT);
	    if( tcsetpgrp(STDIN_FILENO, PGs_table.back()) == -1 ||
	        tcsetpgrp(STDOUT_FILENO, PGs_table.back()) == -1 ||
	        tcsetpgrp(STDERR_FILENO, PGs_table.back()) == -1 )
	    {
		    perror("Failed to set foreground process for command");
        }
        waitpid(PGs_table.back(), &status, WUNTRACED);
        if(!WIFSTOPPED(status))
        {
            PGs_cmd.pop_back();
            PGs_table.pop_back();
        }
	    if( tcsetpgrp(STDIN_FILENO, PGs_table[SHELL_PG_IDX]) == -1 ||
	        tcsetpgrp(STDOUT_FILENO, PGs_table[SHELL_PG_IDX]) == -1 ||
	        tcsetpgrp(STDERR_FILENO, PGs_table[SHELL_PG_IDX]) == -1 )
	    {
	        perror("Failed to set foreground process for shell\n");
        }
        return MAGIC_BUILDIN;
    }
    if((strcmp(argv_store[0],"bg") == 0))
    {
        kill(-PGs_table.back(),SIGCONT);
        PGs_cmd.pop_back();
        PGs_table.pop_back();
        return MAGIC_BUILDIN;
    }
    pid_t pid;
    if ((pid = fork ()) == 0)
    {
        if(is_first)
        {
            if(setpgid(0,0))
            {
                perror("setpgid failed");
            }
        }else
        {
            if(setpgid(0,PGs_table.back()))
            {
                perror("setpgid failed");
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
        if( sigprocmask(SIG_UNBLOCK, &SignalSet, NULL) == -1 )
        {
            perror("Failed to change signal mask.");
        }
        if(execvp(argv_store[0], argv_store) == -1)
        {
            fprintf(stderr,"exec():%s  failed,errno=%d\n",argv_store[0],errno);
            _exit(EXIT_FAILURE); // If child fails
        }
    }else if(pid > 0 && is_first)
    {
       PGs_table.push_back(pid); 
    }
    globfree(&glob_result);
    return pid;//child never comes here
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
    BACKGROUND_MODE = 0;
    WaitForChildren = 1;
    if( sigprocmask(SIG_BLOCK, &SignalSet, NULL) == -1 )
    {
			perror("Failed to change signal mask.");
	}
    PGs_cmd.push_back(cmd);
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
    for(int i = 0;i < cmd_idx;i++)//the last command should output to the original STDOUT_FILENO
    {
        pipe(pipe_fd);
        sub_command(in,pipe_fd[PIPE_OUT_FD],argv_store[i],i==0);
        close(pipe_fd[PIPE_OUT_FD]);
        in = pipe_fd[PIPE_IN_FD];
    }
    //last command in pipeline
    int pid = MAGIC_INVALID_CMD;
    int fd_in;
    int fd_out;
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);
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
    if(argv_store[cmd_idx][0] != NULL)
    {
        if(strcmp(argv_store[cmd_idx][0],"exit") == 0 && cmd_idx == 0)
        {
            exit(EXIT_SUCCESS);
        }
        if( sigprocmask(SIG_UNBLOCK, &SignalSet, NULL) == -1 )
        {
            perror("Failed to change signal mask.");
        }
        pid = sub_command(in,STDOUT_FILENO,argv_store[cmd_idx],cmd_idx==0);
    }
    if(pid == MAGIC_INVALID_CMD)
    {
        return;
    }
    if( pid != MAGIC_BUILDIN && pid != MAGIC_INVALID_CMD)
    {
        if(REDIRECT_IN)
        {
            dup2(stdin_copy,STDIN_FILENO);
            close(stdin_copy);
        }
        if(REDIRECT_OUT)
        {
            dup2(stdout_copy,STDOUT_FILENO);
            close(stdout_copy);
        }
        if(!BACKGROUND_MODE)
        {
	        if( tcsetpgrp(STDIN_FILENO, PGs_table.back()) == -1 ||
	            tcsetpgrp(STDOUT_FILENO, PGs_table.back()) == -1 ||
	            tcsetpgrp(STDERR_FILENO, PGs_table.back()) == -1 )
	        {
		        perror("Failed to set foreground process for command");
            }
            int status;
            /*
            while((waitpid(pid, &status, WNOHANG) <= 0) && WaitForChildren)
            {
            }
            */
            waitpid(pid, &status, WUNTRACED);
            if(!WIFSTOPPED(status))
            {
                PGs_cmd.pop_back();
                PGs_table.pop_back();
            }
        }
	    if( tcsetpgrp(STDIN_FILENO, PGs_table[SHELL_PG_IDX]) == -1 ||
	        tcsetpgrp(STDOUT_FILENO, PGs_table[SHELL_PG_IDX]) == -1 ||
	        tcsetpgrp(STDERR_FILENO, PGs_table[SHELL_PG_IDX]) == -1 )
	    {
	        perror("Failed to set foreground process for shell\n");
        }
        curr_PG++;
    }else//shell build-in
    {
        PGs_cmd.pop_back();
    }
    fflush(NULL);
}

int main(void)
{
 	if(signal(SIGINT, KillChildren) == SIG_ERR ||
	   signal(SIGQUIT, KillChildren) == SIG_ERR)
    {
        perror("SIGINT and SIGQUIT register failed");
    }

	//ignore fg/bg process signal
	if( signal(SIGTTIN, SIG_IGN) == SIG_ERR ||
	    signal(SIGTTOU, SIG_IGN) == SIG_ERR  )
    {
        perror("SIGTTIN,SIGTTOU  register failed");
    }

    if( sigemptyset(&SignalSet) == -1 ||
	    sigaddset(&SignalSet, SIGINT) == -1 ||
	    sigaddset(&SignalSet, SIGQUIT) == -1 )
    {
		perror("Failed to set signal set.");
	}
    if(signal(SIGTSTP, SuspendChildren) == SIG_ERR)
    {
        perror("support ctrl+z failed in child and shell");
    }

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        perror("getcwd() error");
    }
    
    size_t buf_len = 2048;
    char *cmd_buf = (char*)malloc(buf_len);
    if( cmd_buf == NULL)
    {
        perror("Unable to allocate buffer");
        exit(1);
    }
    if(setpgid(0,0) == -1)
    {
        perror("Shell failed to become new pg");
    }
    PGs_table.clear();
    PGs_cmd.clear();
    PGs_table.push_back(getpid());//first pid is also the pgid
    PGs_cmd.push_back("shell-prompt");
    curr_PG = SHELL_PG_IDX + 1;
    while(1)
    {
        printf("shell-prompt$ ");
		fflush(NULL);
        getline(&cmd_buf, &buf_len, stdin);
        run(cmd_buf);
        memset(cmd_buf,0,buf_len);
    }
    free(cmd_buf);
}
