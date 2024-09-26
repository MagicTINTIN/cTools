#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

std::string urlEncode(const std::string &input)
{
    std::ostringstream encoded;
    encoded.fill('0');
    encoded << std::hex;

    for (char c : input)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~' || c == '?')
        {
            encoded << c;
        }
        else if (c == ' ')
        {
            encoded << '+';
        }
        else
        {
            encoded << '%' << std::setw(2) << int(static_cast<unsigned char>(c));
        }
    }

    return encoded.str();
}

std::string string_to_hex(const std::string &input)
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
    if (argc > 2)
    {
        std::cerr << argv[0] << " take an optional url as arg\n";
        return 1;
    }

    std::string value("");
    if (argc == 1)
    {
        std::string s;
        std::cout << "Paste text to copy here (Press Enter and then Ctrl+C or Ctrl+D to end copy):\n";
        while (getline(std::cin, s))
        {
            if (s.empty())
                break;
            value += "\n" + s;
        }
    }
    else value = std::string(argv[1]);

    std::cout << urlEncode(value) << std::endl;
    return 0;
}
