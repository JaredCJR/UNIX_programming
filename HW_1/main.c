#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define IS_OUTPUTTED      0
#define IS_NOT_OUTPUTTED  1

#define ADDR_MAX_LEN      40

#define OPT_RET_TCP       1
#define OPT_RET_UDP       2

#define TCP_v4            (uint32_t)0x0001
#define TCP_v6            (uint32_t)0x0002
#define UDP_v4            (uint32_t)0x0004
#define UDP_v6            (uint32_t)0x0008

char filter_string[255];

typedef struct{
    int connection_type;
    char local_addr[ADDR_MAX_LEN];
    char rem_addr[ADDR_MAX_LEN];
    int pid;
    char pid_name[255];
}CONNECTION_INFO;

typedef struct CONNECTION_HOUSE{
    CONNECTION_INFO info;
    struct CONNECTION_HOUSE *p2next;
}CONNECTION_HOUSE;


CONNECTION_HOUSE *house_head = NULL;
CONNECTION_HOUSE *house_tail = NULL;

static void info_parser(int connection_type,char *p2info,uint32_t type)
{
    char *p2local;
    char *p2rem;
    char separate_string[] = " ";
    /*ignore list number*/
    p2local = strtok (p2info, separate_string);
    /*get address*/
    p2local = strtok (NULL, separate_string);
    p2rem = strtok (NULL, separate_string);
    if(house_head == NULL)
    {
        house_head = (CONNECTION_HOUSE*)malloc(sizeof(CONNECTION_HOUSE));
        snprintf(house_head->info.local_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2local);
        snprintf(house_head->info.rem_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2rem);
        house_tail = house_head;
    }else
    {
        house_tail->p2next = (CONNECTION_HOUSE*)malloc(sizeof(CONNECTION_HOUSE));
        house_tail = house_tail->p2next;
        snprintf(house_tail->info.local_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2local);
        snprintf(house_tail->info.rem_addr,ADDR_MAX_LEN*sizeof(char),"%s",p2rem);
    }
    house_tail->info.connection_type = type;
    house_tail->p2next = NULL;
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
    char *p2info= 0;
    ssize_t line_len = 0;
    do {
        line_len = getline (&p2info, &size_in_line, p2file);
        if (line_len < 0)
        {
            break;
        }
        /*TODO:add pid and arg info*/
        if(first)
        {
            /*get rid of head line*/
            line_len = getline (&p2info, &size_in_line, p2file);
            first = 0;
        }
        info_parser(connection_type,p2info,type);
    } while (!feof (p2file));
    fclose(p2file);
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

static void printf_hex2dec_v4(char *addr)
{
    char buffer[25];
    int result[5] = {0};
    /*get IP readable*/
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
    printf("%-24s",buffer);
}

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

static void v6factory(char *buffer,uint32_t addr_frag)
{
    char buffer_tmp[32];
    dec2str(buffer_tmp,addr_frag);
    strcat(buffer,buffer_tmp);
}

static void printf_hex2dec_v6(char *addr)
{
    char buffer[100] = {};
    uint32_t result[9] = {0};
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
    printf("%-24s",buffer);
}


static void print_v4_info(uint32_t type)
{
    CONNECTION_HOUSE *temp = house_head;
    char type_tcp[] = "tcp";
    char type_udp[] = "udp";
    char *target_type;
    if(type == TCP_v4)
    {
        target_type = type_tcp;
    }else
    {
        target_type = type_udp;
    }
    while(temp != NULL)
    {
        if((temp->info.connection_type & TCP_v4) || (temp->info.connection_type & UDP_v4))
        {
            //printf("%-6s%-40s%-40s\n",target_type,temp->info.local_addr,temp->info.rem_addr);
            printf("%-6s",target_type);
            printf_hex2dec_v4(temp->info.local_addr);
            printf_hex2dec_v4(temp->info.rem_addr);
            printf("\n");
        }
        temp = temp->p2next;
    }
}

static void print_v6_info(uint32_t type)
{
    CONNECTION_HOUSE *temp = house_head;
    char type_tcp[] = "tcp6";
    char type_udp[] = "udp6";
    char *target_type;
    if(type == TCP_v6)
    {
        target_type = type_tcp;
    }else
    {
        target_type = type_udp;
    }
    while(temp != NULL)
    {
        if((temp->info.connection_type & TCP_v6) || (temp->info.connection_type & UDP_v6))
        {
            //printf("%-6s%-40s%-40s\n",target_type,temp->info.local_addr,temp->info.rem_addr);
            printf("%-6s",target_type);
            printf_hex2dec_v6(temp->info.local_addr);
            printf_hex2dec_v6(temp->info.rem_addr);
            printf("\n");
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
        print_v4_info(TCP_v4);
        print_v6_info(TCP_v6);
        printf("\n");
    }
    if(all_tpye & UDP_v4)
    {
        printf("List of UDP connections:\n");
        printf("Proto Local Address           Foreign Address         PID/Program name and arguments\n");
        print_v4_info(UDP_v4);
        print_v6_info(UDP_v6);
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

    if(*argv[argc-1] != '-')
    {
        snprintf(filter_string,255,"%s",argv[argc-1]);
        printf("filter: %s\n",filter_string);
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
