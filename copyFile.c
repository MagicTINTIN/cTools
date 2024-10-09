#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFSIZE 4096

static void copy_file(const char *src, const char *dst)
{
    int  fdsrc, fddst;
    char buffer[BUFSIZE];
    int nchar;
    fdsrc = open(src, O_RDONLY);
    fddst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    while ((nchar = read(fdsrc, buffer, BUFSIZE)))
    {
        write(fddst, buffer, nchar);
    }
}

int main(int argc, char const *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Need src and dest\n");
        exit(1);
    }
    copy_file(argv[1], argv[2]);
    return 0;
}
