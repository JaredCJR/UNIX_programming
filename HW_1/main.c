#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define IS_OUTPUTTED      0
#define IS_NOT_OUTPUTTED  1

#define ADDR_MAX_LEN      40

#define OPT_RET_TCP       1
#define OPT_RET_UDP       2

#define TCP_v4            (uint32_t)0x0001
#define TCP_v6            (uint32_t)0x0002
#define UDP_v4            (uint32_t)0x0004
#define UDP_v6            (uint32_t)0x0008

#define MAX_INODE_IN_PID  100
#define UNUSED_IDX        -1

#define REMOTE_ADDR       1
#define LOCAL_ADDR        2

#define FILTER_ON         1
#define FILTER_OFF        0

char filter_string[255];
int filter_mode = FILTER_OFF;

typedef struct{
    uint32_t pid;
    uint32_t inode_num[MAX_INODE_IN_PID];
    int inode_num_idx;
    char pid_env[255];/*cmd line and args*/
}PROC_INFO;

typedef struct PROC_HOUSE{
    PROC_INFO pid_info;
    struct PROC_HOUSE *p2next;
}PROC_HOUSE;

typedef struct{
    int connection_type;
    char local_addr[ADDR_MAX_LEN];
    char rem_addr[ADDR_MAX_LEN];
    uint32_t inode_num;
    PROC_INFO *pid_info;
}CONNECTION_INFO;

typedef struct CONNECTION_HOUSE{
    CONNECTION_INFO info;
    struct CONNECTION_HOUSE *p2next;
}CONNECTION_HOUSE;

CONNECTION_HOUSE *conn_house_head = NULL;
CONNECTION_HOUSE *conn_house_tail = NULL;
PROC_HOUSE *pid_house_head = NULL;
PROC_HOUSE *pid_house_tail = NULL;

static void str_reverse(char *str)
{
    int start = 0;
    int end = strlen(str)-1;
    char tmp;
    while(start < end)
    {
        /*swap*/
        tmp = str[end];
        str[end] = str[start];
        str[start] = tmp;
        start++;
        end--;
    }
}

static void dec2str(char *buffer,uint32_t num)
{
    int i = 0;
    if(num == 0)
    {
        buffer[i++] = '0';
        buffer[i] = '\0';
        return;
    }
    while(num != 0)
    {
        buffer[i++] = '0' + (num % 10);
        num /= 10;
    }
    buffer[i] = '\0';
    str_reverse(buffer);
}

static uint32_t str2dec(char *str)
{
    uint32_t result = 0;
    while(*str != '\0')
    {
        if((*str > '9') || (*str < '0'))
        {
            break;
        }
        result  = result*10 + (*str - '0');
        str++;
    }
    return result;
}

static uint32_t ONEchar2dec(char *p2byte)
{
    return (*p2byte >= 'A') ? (10 + (*p2byte-'A')) : (*p2byte - '0');
}

static uint32_t TWOchar2dec(char *byte_1)
{
    uint32_t result = 0;
    result = ONEchar2dec(byte_1);
    result = result * 16 + ONEchar2dec(byte_1+1);
    return result;
}

static uint32_t FOURchar2dec(char *byte_1)
{
    uint32_t result = 0;
    result = TWOchar2dec(byte_1);
    result = (result << 8) + TWOchar2dec(byte_1+2);
    return result;
}

static void store_inode(PROC_HOUSE *proc,uint32_t inode_num)
{
    proc->pid_info.inode_num_idx++;
    proc->pid_info.inode_num[proc->pid_info.inode_num_idx] = inode_num;
    if(proc->pid_info.inode_num_idx >= MAX_INODE_IN_PID)
    {
        fprintf(stderr,"inode is too much for the storage\n");
    }
}

/*parse until first '/' or the string head*/
static void parse_cmdline(char *input)
{
    int end = strlen(input)-1;
    for(int i = end;i >= 0;i--)
    {
        if(input[i] == '/')
        {
            snprintf(input,end-i+1,"%s",&input[i+1]);
            return;
        }
    }
}

static void create_pid_db(char *pid)
{
    char buffer[255];
    /*get pid path*/
    snprintf(buffer,sizeof(buffer),"/proc/%d/cmdline",str2dec(pid));
    FILE *p2file = fopen(buffer, "r");
    size_t size_in_line = 0;
    char *p2info= NULL;
    ssize_t line_len = 0;
    /*get info*/
    line_len = getline (&p2info, &size_in_line, p2file);
    if (line_len >= 0)
    {
        if(pid_house_head == NULL)
        {
            pid_house_head = (PROC_HOUSE*)malloc(sizeof(PROC_HOUSE));
            pid_house_tail = pid_house_head;
        }else
        {
            pid_house_tail->p2next = (PROC_HOUSE*)malloc(sizeof(PROC_HOUSE));
            pid_house_tail = pid_house_tail->p2next;
        }
        pid_house_tail->p2next = NULL;
        pid_house_tail->pid_info.pid = str2dec(pid);
        pid_house_tail->pid_info.inode_num_idx = UNUSED_IDX;
        parse_cmdline(p2info);
        snprintf(pid_house_tail->pid_info.pid_env,sizeof(pid_house_tail->pid_info.pid_env),"%s",p2info);
        //printf("%d  : %s\n",pid_house_tail->pid_info.pid,pid_house_tail->pid_info.pid_env);
    }
    free(p2info);
    fclose(p2file);

    /*get inode number*/
    snprintf(buffer,sizeof(buffer),"/proc/%d/fd/",str2dec(pid));
    char true_path[255];
    char fd_content[255];
    DIR *dir;
    struct dirent *ent;
    uint32_t inode_num;
    if ((dir = opendir (buffer)) != NULL) 
    {
        /* get all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) 
        {
            snprintf(true_path,sizeof(true_path),"%s%s",buffer,ent->d_name);
            memset(fd_content,0,sizeof(fd_content));
            if (readlink(true_path,fd_content,sizeof(fd_content)) >= 0)
            {
                /*we are looking for inode string, such as socket:[46683] */
                if((strlen(fd_content) > 9)&&(strncmp(fd_content,"socket:[",8) == 0))
                {
                    inode_num = str2dec(fd_content+8);
                    //printf("%d\n",inode_num);
                    store_inode(pid_house_tail,inode_num);
                }
            }else
            {
                /*ignore those are not linking info*/
            }
        }
        closedir (dir);
    } else 
    {
        /* could not open directory */
        perror ("");
    }
}


static void pid_factory(void)
{
    /*only need once*/
    static int create_dir_db = 0;
    if(create_dir_db == 0)
    {
        DIR *dir;
        struct dirent *ent;
        char dir_name[255];
        if ((dir = opendir ("/proc")) != NULL) 
        {
            /* get all the files and directories within directory */
            while ((ent = readdir (dir)) != NULL) 
            {
                snprintf(dir_name,sizeof(dir_name),"%s",ent->d_name);
                /*get name with "digit" as head*/
                if((dir_name[0]-'0' > 0) && (dir_name[0]-'0' <= 9))
                {
                    //printf("%s\n",dir_name);
                    create_pid_db(dir_name);
                }
            }
            closedir (dir);
            /*only do once*/
            create_dir_db = 1;
        } else 
        {
            /* could not open directory */
            perror ("");
        }
    }
}

/*get owner from inode num*/
void get_connection_owner(CONNECTION_HOUSE *con)
{
    uint32_t target_inode = con->info.inode_num;
    PROC_HOUSE *tmp = pid_house_head;
    while(tmp != NULL)
    {
        for(int i = 0;i <= tmp->pid_info.inode_num_idx;i++)
        {
            if(tmp->pid_info.inode_num[i] == target_inode)
            {
                con->info.pid_info = &tmp->pid_info;
                return;
            }
        }
        /*next round*/
        tmp = tmp->p2next;
    }
    //fprintf(stderr,"DO you have root permission to access /proc filesystem?\n");
}

static void addr_hex2dec_v4(CONNECTION_HOUSE *conn,int type)
{
    char buffer[30];
    int result[5] = {0};
    char *addr;
    if(type == LOCAL_ADDR)
    {
        addr = conn->info.local_addr;
    }else
    {
        addr = conn->info.rem_addr;
    }
    /*get readable local IP*/
    result[0] = TWOchar2dec(addr+6);
    result[1] = TWOchar2dec(addr+4);
    result[2] = TWOchar2dec(addr+2);
    result[3] = TWOchar2dec(addr+0);

    /*get port readable*/
    result[4] = FOURchar2dec(addr+9);

    if(result[4] == 0)
    {
        snprintf(buffer,sizeof(buffer),"%d.%d.%d.%d:*",result[0],result[1],result[2],result[3]);
    }else
    {
        snprintf(buffer,sizeof(buffer),"%d.%d.%d.%d:%d",result[0],result[1],result[2],result[3],result[4]);
    }
    
    if(type == LOCAL_ADDR)
    {
        snprintf(conn->info.local_addr,sizeof(conn->info.local_addr),"%s",buffer);
    }else
    {
        snprintf(conn->info.rem_addr,sizeof(conn->info.local_addr),"%s",buffer);
    }
}

static void v6factory(char *buffer,uint32_t addr_frag)
{
    char buffer_tmp[32];
    dec2str(buffer_tmp,addr_frag);
    strcat(buffer,buffer_tmp);
}

static void addr_hex2dec_v6(CONNECTION_HOUSE *conn,int type)
{
    char buffer[100] = {};
    uint32_t result[9] = {0};
    char *addr;
    if(type == LOCAL_ADDR)
    {
        addr = conn->info.local_addr;
    }else
    {
        addr = conn->info.rem_addr;
    }
    /*get IP readable*/
    result[0] = FOURchar2dec(addr+28);
    result[1] = FOURchar2dec(addr+24);
    result[2] = FOURchar2dec(addr+20);
    result[3] = FOURchar2dec(addr+16);
    result[4] = FOURchar2dec(addr+12);
    result[5] = FOURchar2dec(addr+8);
    result[6] = FOURchar2dec(addr+4);
    result[7] = FOURchar2dec(addr+0);
    for(int i = 0;i < 8;i++)
    {
        v6factory(buffer,result[i]);
        strcat(buffer,":");
    }

    /*get port readable*/
    result[8] = FOURchar2dec(addr+33);

    if(result[8] == 0)
    {
        snprintf(buffer+strlen(buffer),sizeof(buffer),"*");
    }else
    {
        snprintf(buffer+strlen(buffer),sizeof(buffer),"%d",result[8]);
    }

    if(type == LOCAL_ADDR)
    {
        snprintf(conn->info.local_addr,sizeof(conn->info.local_addr),"%s",buffer);
    }else
    {
        snprintf(conn->info.rem_addr,sizeof(conn->info.local_addr),"%s",buffer);
    }
}
static void info_parser(int connection_type,char *p2info,uint32_t type)
{
    char *p2local;
    char *p2rem;
    char *p2inode;
    char separate_string[] = " ";
    /*ignore list number*/
    p2local = strtok (p2info, separate_string);
    /*get address*/
    p2local = strtok (NULL, separate_string);
    p2rem = strtok (NULL, separate_string);
    if(conn_house_head == NULL)
    {
        conn_house_head = (CONNECTION_HOUSE*)malloc(sizeof(CONNECTION_HOUSE));
        conn_house_tail = conn_house_head;
    }else
    {
        conn_house_tail->p2next = (CONNECTION_HOUSE*)malloc(sizeof(CONNECTION_HOUSE));
        conn_house_tail = conn_house_tail->p2next;
    }
    conn_house_tail->p2next = NULL;
    conn_house_tail->info.connection_type = type;
    snprintf(conn_house_tail->info.local_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2local);
    snprintf(conn_house_tail->info.rem_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2rem);
    if((connection_type & TCP_v4) || (connection_type & UDP_v4))
    {
        addr_hex2dec_v4(conn_house_tail,REMOTE_ADDR);
        addr_hex2dec_v4(conn_house_tail,LOCAL_ADDR);
    }else if((connection_type & TCP_v6) || (connection_type & UDP_v6))
    {
        addr_hex2dec_v6(conn_house_tail,REMOTE_ADDR);
        addr_hex2dec_v6(conn_house_tail,LOCAL_ADDR);
    }else
    {
        fprintf(stderr,"Transform to human-readable address type error!\n");
        exit(EXIT_FAILURE);
    }

    /*get inode number*/
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    p2inode = strtok (NULL, separate_string);
    conn_house_tail->info.inode_num = str2dec(p2inode);

    /*get pid about the connection*/
    conn_house_tail->info.pid_info = NULL;
    get_connection_owner(conn_house_tail);
}

void create_db(int connection_type)
{
    int first = 1;
    char p2dst[20];
    uint32_t type = 0;
    switch(connection_type)
    {
        case TCP_v4:
            type = TCP_v4;
            snprintf(p2dst,20,"%s","/proc/net/tcp");
            break;
        case TCP_v6:
            type = TCP_v6;
            snprintf(p2dst,20,"%s","/proc/net/tcp6");
            break;
        case UDP_v4:
            type = UDP_v4;
            snprintf(p2dst,20,"%s","/proc/net/udp");
            break;
        case UDP_v6:
            type = UDP_v6;
            snprintf(p2dst,20,"%s","/proc/net/udp6");
            break;
        default:
            fprintf(stderr,"Create db with unknown type.\n");
            exit(EXIT_FAILURE);
            break;
    }
    FILE *p2file = fopen(p2dst, "r");
    size_t size_in_line = 0;
    char *p2info= NULL;
    ssize_t line_len = 0;
    do {
        line_len = getline (&p2info, &size_in_line, p2file);
        if (line_len < 0)
        {
            break;
        }
        if(first)
        {
            /*get rid of head line*/
            line_len = getline (&p2info, &size_in_line, p2file);
            first = 0;
            /*create pid info*/
            pid_factory();
        }
        info_parser(connection_type,p2info,type);
    } while (!feof (p2file));
    fclose(p2file);
    free(p2info);
}

void get_connection(uint32_t all_tpye)
{
    if(all_tpye & TCP_v4)
    {
        create_db(TCP_v4);
    }
    if(all_tpye & TCP_v6)
    {
        create_db(TCP_v6);
    }
    if(all_tpye & UDP_v4)
    {
        create_db(UDP_v4);
    }
    if(all_tpye & UDP_v6)
    {
        create_db(UDP_v6);
    }
}

void filter_factory(char *input)
{
    if(filter_mode == FILTER_ON)
    {
        /*target: filter_string*/
        if(strstr(input,filter_string) == NULL)
        {
            return;
        }
    }
    printf("%s",input);
}

static void print_v4_v6_info(uint32_t type)
{
    CONNECTION_HOUSE *temp = conn_house_head;
    char buffer[255];
    char type_tcp[] = "tcp";
    char type_udp[] = "udp";
    char type_tcp6[] = "tcp6";
    char type_udp6[] = "udp6";
    char *target_type;
    if(type == TCP_v4)
    {
        target_type = type_tcp;
    }else if(type == TCP_v6)
    {
        target_type = type_tcp6;
    }else if(type == UDP_v4)
    {
        target_type = type_udp;
    }else
    {
        target_type = type_udp6;
    }

    while(temp != NULL)
    {
        if(temp->info.connection_type & type)
        {
            snprintf(buffer,sizeof(buffer),"%-6s",target_type);
            snprintf(buffer+6,sizeof(buffer),"%-24s",temp->info.local_addr);
            snprintf(buffer+6+24,sizeof(buffer),"%-24s",temp->info.rem_addr);
            if(temp->info.pid_info != NULL)
            {
                snprintf(buffer+6+24+24,sizeof(buffer),"%d/%s\n",temp->info.pid_info->pid,temp->info.pid_info->pid_env);
            }else
            {
                snprintf(buffer+6+24+24,sizeof(buffer),"-\n");
            }
            filter_factory(buffer);
        }
        temp = temp->p2next;
    }
}


void print_info(uint32_t all_tpye)
{
    if(all_tpye & TCP_v4)
    {
        printf("List of TCP connections:\n");
        printf("Proto Local Address           Foreign Address         PID/Program name and arguments\n");
        print_v4_v6_info(TCP_v4);
        print_v4_v6_info(TCP_v6);
        printf("\n");
    }
    if(all_tpye & UDP_v4)
    {
        printf("List of UDP connections:\n");
        printf("Proto Local Address           Foreign Address         PID/Program name and arguments\n");
        print_v4_v6_info(UDP_v4);
        print_v4_v6_info(UDP_v6);
        printf("\n");
    }
}

int main(int argc, char *argv[])
{
    static int output_status = IS_NOT_OUTPUTTED;
    struct option long_option[] = {
        {"tcp", no_argument, 0, OPT_RET_TCP},
        {"udp", no_argument, 0, OPT_RET_UDP},
    };
    filter_mode = FILTER_OFF;

    if((*argv[argc-1] != '-') && (argc > 1))
    {
        snprintf(filter_string,255,"%s",argv[argc-1]);
        filter_mode = FILTER_ON;
    }

    int opt_ret;
    while( -1 != (opt_ret = getopt_long(argc,argv,"t,u",long_option,NULL)))
    {
        output_status = IS_OUTPUTTED;
        switch(opt_ret)
        {
            case 't':
            case OPT_RET_TCP:
                get_connection(TCP_v4 | TCP_v6);
                print_info(TCP_v4 | TCP_v6);
                break;
            case 'u':
            case OPT_RET_UDP:
                get_connection(UDP_v4 | UDP_v6);
                print_info(UDP_v4 | UDP_v6);
                break;
            default:
                printf("undefined option\n");
                printf("usage: $ ./hw1 [-t|--tcp] [-u|--udp] [filter-string]\n");
                printf("NOTE: The maximum size for filter_string is 255\n");
                break;
        }
    }
    if(output_status != IS_OUTPUTTED)
    {
        get_connection(TCP_v4 | TCP_v6 | UDP_v4 | UDP_v6);
        print_info(TCP_v4 | TCP_v6 | UDP_v4 | UDP_v6);
    }
    return EXIT_SUCCESS;
}
