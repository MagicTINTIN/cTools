#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char const *argv[])
{
    srand(time(NULL)); // Initialization, should only be called once.
    int r = rand();    // Returns a pseudo-random integer between 0 and RAND_MAX.

    int totalGuess = 0;
    int correct = 0;

    char words[26][128] = {
        "alfa",
        "bravo",
        "charlie",
        "delta",
        "echo",
        "foxtrot",
        "golf",
        "hotel",
        "india",
        "juliett",
        "kilo",
        "lima",
        "mike",
        "november",
        "oscar",
        "papa",
        "quebec",
        "romeo",
        "sierra",
        "tango",
        "uniform",
        "victor",
        "whiskey",
        "xray",
        "yankee",
        "zulu"};

    while (totalGuess < 26 || correct * 100 < totalGuess * 95)
    {
        r = rand() % 26;
        printf("%c: ", (r + 'a'));
        char input[256];
        scanf("%s", input);
        if (strncmp(input, words[r], 100) == 0)
        {
            totalGuess++;
            printf("Vrai!\n");
        }
        else
        {
            printf("Faux! -> %s\n", words[r]);
        }
    }

    return 0;
}
