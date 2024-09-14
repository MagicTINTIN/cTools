#include <string>
#include <iostream>
#include <fstream>

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        std::cerr << argv[0] << " takes 3 params (filepath in/out & max_size)\n";
        return 1;
    }
    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    std::string line;
    long unsigned int maxAllowed = atoi(argv[3]);
    unsigned int lines(0), copiedLines(0);
    while (std::getline(infile, line))
    {
        lines++;
        // std::cout << line << "\n";
        if (line.size() < maxAllowed) {
            outfile << line << "\n";
            copiedLines++;
        }
    }
    printf("Finished. %d/%d lines copied.\n", copiedLines, lines);
    return 0;
}
