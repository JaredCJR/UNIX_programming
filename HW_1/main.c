#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdint.h>

#define IS_OUTPUTTED      0
#define IS_NOT_OUTPUTTED  1

#define OPT_RET_TCP  1
#define OPT_RET_UDP  2

#define TCP_v4            (uint32_t)0x0001
#define TCP_v6            (uint32_t)0x0002
#define UDP_v4            (uint32_t)0x0004
#define UDP_v6            (uint32_t)0x0008

char filter_string[255];

typedef struct{
    int connection_type;
    char local_addr[38];
    char rem_addr[38];
    int pid;
    char pid_name[255];
}CONNECTION_INFO;

void create_db(int connection_type)
{
    char p2dst[20];
    switch(connection_type)
    {
        case TCP_v4:
            snprintf(p2dst,20,"%s","/proc/net/tcp");
            break;
        case TCP_v6:
            snprintf(p2dst,20,"%s","/proc/net/tcp6");
            break;
        case UDP_v4:
            snprintf(p2dst,20,"%s","/proc/net/udp");
            break;
        case UDP_v6:
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
        /*TODO:parse info*/
        printf("%s",p2info);
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
                break;
            case 'u':
            case OPT_RET_UDP:
                get_connection(UDP_v4 | UDP_v6);
                break;
            default:
                printf("undefined option\n");
                printf("usage: $ ./hw1 [-t|--tcp] [-u|--udp] [filter-string]\n");
                printf("NOTE: The maximum size for filter_string is 255\n");
                break;/*do nothing*/
        }
    }
    if(output_status != IS_OUTPUTTED)
    {
        get_connection(TCP_v4 | TCP_v6 | UDP_v4 | UDP_v6);
    }
    return EXIT_SUCCESS;
}
