#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Need a file\n");
        exit(1);
    }
 
    FILE* demo;
    int display;
 
    demo = fopen(argv[1], "r");
 
    // loop to extract every characters
    while (1) {
        // reading file
        display = fgetc(demo);
 
        // end of file indicator
        if (feof(demo))
            break;
 
        // displaying every characters
        printf("%c", display);
    }
 
    // closes the file pointed by demo
    fclose(demo);
 
    return 0;
}