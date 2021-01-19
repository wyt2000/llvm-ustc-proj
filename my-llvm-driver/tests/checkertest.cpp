#include <stdio.h>

FILE *open(char *file)
{
    return fopen(file, "r");
}

void f1(FILE *f)
{
    // do something...
    fclose(f);
}

void f2(FILE *f)
{
    // do something...
    fclose(f);
}

int main()
{
    FILE *f = open("foo");
    f1(f);
    f2(f);
    return 0;
}