#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include "test_func.h"

/*only for testing*/

void __get_uid()
{
    printf("UID = %d\n", getuid());
}

void __fputc()
{
    fputc('C',stdout);
    fputc('\n',stdout);
}
