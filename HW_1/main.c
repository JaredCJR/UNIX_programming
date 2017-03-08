#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#define OPT_RET_TCP  1
#define OPT_RET_UDP  2

char filter_string[255];

int main(int argc, char *argv[])
{
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
        switch(opt_ret)
        {
            case 't':
            case OPT_RET_TCP:
                printf("tcp option\n");
                break;
            case 'u':
            case OPT_RET_UDP:
                printf("udp option\n");
                break;
            default:
                printf("undefined option\n");
                printf("usage: $ ./hw1 [-t|--tcp] [-u|--udp] [filter-string]\n");
                printf("NOTE: The maximum size for filter_string is 255\n");
                break;/*do nothing*/
        }
    }
    return EXIT_SUCCESS;
}
