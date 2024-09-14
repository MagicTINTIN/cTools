#include <string>
#include <iostream>
#include <fstream>

std::string replaceChar(std::string s, char c1, char c2)
{
    for (long unsigned int i = 0; i < s.length(); i++) 
    {
        if (s[i] == c1)
            s[i] = c2;
        // else if (s[i] == c2)
        //     s[i] = c1;
    }
    return s;
}

std::string deleteChar(std::string s, char c1)
{
    std::string ret("");
    for (long unsigned int i = 0; i < s.length(); i++) 
    {
        if (s[i] != c1)
            ret+=s[i];
    }
    return ret;
}

std::string string_to_hex(const std::string& input)
{
    static const char hex_digits[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (unsigned char c : input)
    {
        output.push_back(hex_digits[c >> 4]);
        output.push_back(hex_digits[c & 15]);
    }
    return output;
}

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
        std::string cleaned = deleteChar(line, 13);// deleteChar(, ' ');
        // std::cout << line << "\n";
        if (cleaned.size() <= maxAllowed) {
            outfile << cleaned << "\n";
            copiedLines++;
            // std::cout << cleaned << ": 0x" << string_to_hex(cleaned) << "\n";
        }
    }
    printf("Finished. %d/%d lines copied.\n", copiedLines, lines);
    return 0;
}
