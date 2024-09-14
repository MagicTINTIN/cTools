#include <string>
#include <iostream>
#include <fstream>

int main(int argc, char const *argv[])
{
    if (argc != 2) {
        std::cerr << argv[0] << " takes 1 more argument (file path)\n";
        return 1;
    }
    std::ifstream infile(argv[1]);
    std::string line;
    long unsigned int max = 0;
    unsigned int lines = 0;
    while (std::getline(infile, line))
    {
        lines++;
        // std::cout << line << "\n";
        if (line.size() > max)
            max = line.size();
    }
    printf("MAX LINE SIZE: %ld (over %d lines)\n", max, lines);
    return 0;
}
