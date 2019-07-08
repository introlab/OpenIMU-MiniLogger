#include <stdio.h>
#include <output_export.h>



void output_start(unsigned int baudrate)
{
    ;
}

void output_char(int c)
{
    putchar(c);
}

void output_flush(void)
{
    fflush(stdout);
}

void output_complete(void)
{
   ;
}