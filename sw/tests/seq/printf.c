#include <math.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    unsigned i;
    for (i=0; i<argc+5; i++)
        printf(" %g", sqrt(i));

    printf("d");
    printf("%d: Hello World%5d% d%c%u%x%04d%5d\n", 0, -123, 45, '$', -294967296, -16, -1, -89);

    return 0;
}
