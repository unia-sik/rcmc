/* Cannot be used baremetal: 
   Must be linked with libplastic instead of libcopper to support argument passing.
 */
#include <math.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned i;
    for (i=0; i<argc; i++)
        printf("%s\n", argv[i]);
    return 0;
}
