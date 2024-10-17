#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define GUESSES 26

int isIn(int n, int tab[], int size)
{
    for (int i = 0; i < size; i++)
    {
        if (tab[i] == n)
            return 1;
    }
    return 0;
}

int min(int a, int b) {return a < b ? a : b;}

int main(int argc, char const *argv[])
{
    srand(time(NULL)); // Initialization, should only be called once.
    // int r = rand();    // Returns a pseudo-random integer between 0 and RAND_MAX.

    int totalGuess = 0;
    int correct = 0;

    char words[GUESSES][128] = {
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
        "zulu"
        };

    int randoms[GUESSES] = {0};
    while (correct < GUESSES)
    {
        if (totalGuess % GUESSES == 0)
        {
            correct = 0;
            totalGuess = 0;
            int values = 0;
            for (; values < GUESSES;)
            {
                int r = rand() % GUESSES;
                if (!isIn(r, randoms, values))
                    randoms[values++] = r;
            }
        }
        printf("%c: ", (randoms[totalGuess % GUESSES] + 'a'));
        char input[256];
        int r = scanf("%s", input);
        if (strlen(input) == strlen(words[randoms[totalGuess % GUESSES]]) && strncmp(input, words[randoms[totalGuess % GUESSES]], min(100, r)) == 0)
        {
            totalGuess++;
            correct++;
            printf("Vrai!\n");
        }
        else
            printf("Faux! -> %s\n", words[randoms[totalGuess % GUESSES]]);
    }

    printf("GG! Streak completed\n");

    return 0;
}
