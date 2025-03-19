#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>

#define DEFAULT_CONTEXT 3
class WordModel
{
private:
    std::vector<size_t> lengthsFrequencies;
    std::vector<std::map<std::string, std::map<std::string, int>>> maps;
    int contextSize;

public:
    WordModel(int contextSize);
    ~WordModel();
    void addStr(std::string str, std::string c);
    void addLength(int length);
};

WordModel::WordModel(int contextSize) : lengthsFrequencies(20), maps(contextSize), contextSize(contextSize)
{
}

WordModel::~WordModel()
{
}

void WordModel::addStr(std::string ctx, std::string c)
{
    size_t sizeOfStr = utf8_length(ctx);
    if (sizeOfStr < contextSize)
        ctx = " " + ctx;
    else
        sizeOfStr--;
    if (maps.at(0).count(ctx))
    {
        if (maps.at(sizeOfStr)[ctx].count(c))
            maps.at(sizeOfStr)[ctx][c]++;
        else
            maps.at(sizeOfStr)[ctx].insert(std::make_pair(c, 1));
    }
    else
    {
        maps.at(sizeOfStr).insert(std::make_pair(ctx, std::map<std::string, int>()));
        maps.at(sizeOfStr)[ctx].insert(std::make_pair(c, 1));
    }
}

void WordModel::addLength(int length)
{
    for (int i = lengthsFrequencies.size(); i < length; i++)
    {
        lengthsFrequencies.emplace_back(0);
    }

    lengthsFrequencies.at(length - 1)++;
}

size_t utf8_length(const std::string &str)
{
    size_t length = 0;
    for (auto it = str.begin(); it != str.end();)
    {
        unsigned char c = *it;
        if ((c & 0x80) == 0x00)
        {
            // 1-byte character
            ++it;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // 2-byte character
            it += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            // 3-byte character
            it += 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            // 4-byte character
            it += 4;
        }
        ++length;
    }
    return length;
}

std::string utf8_char_at(const std::string &str, size_t n)
{
    size_t current_pos = 0;
    size_t byte_index = 0;

    while (current_pos < n && byte_index < str.size())
    {
        unsigned char c = str[byte_index];
        if ((c & 0x80) == 0x00)
        {
            // 1-byte character
            ++byte_index;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // 2-byte character
            byte_index += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            // 3-byte character
            byte_index += 3;
        }
        else if ((c & 0xF8) == 0xF0)
        {
            // 4-byte character
            byte_index += 4;
        }
        ++current_pos;
    }

    if (current_pos == n && byte_index < str.size())
    {
        unsigned char c = str[byte_index];
        if ((c & 0x80) == 0x00)
        {
            // 1-byte character
            return str.substr(byte_index, 1);
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // 2-byte character
            return str.substr(byte_index, 2);
        }
        else if ((c & 0xF0) == 0xE0)
        {
            // 3-byte character
            return str.substr(byte_index, 3);
        }
        else if ((c & 0xF8) == 0xF0)
        {
            // 4-byte character
            return str.substr(byte_index, 4);
        }
    }

    // Return an empty string if the position is out of bounds
    return "";
}

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
            ret += s[i];
    }
    return ret;
}

int main(int argc, char const *argv[])
{
    // std::cout << std::string("être").length() << std::endl;
    // return 1;
    if (argc != 4 && argc != 5)
    {
        std::cerr << argv[0] << " takes 3 params (filepath in/out & number of words generated) [context_size]\n";
        return 1;
    }

    int contextSize = DEFAULT_CONTEXT;
    if (argc == 5)
    {
        contextSize = atoi(argv[4]);
    }
    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    std::string line;

    long unsigned int maxAllowed = atoi(argv[3]);

    unsigned int lines(0);
    while (std::getline(infile, line))
    {
        lines++;
        printf("Str=%s : len=%ld, size=%ld, utf8=%ld | ", line.c_str(), line.length(), line.size(), utf8_length(line));
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        printf("Slen=%ld, size=%ld, utf8=%ld\n", line.length(), line.size(), utf8_length(line));
        std::string cleaned = deleteChar(line, 13);
        bool alpha = true;
        for (long unsigned int i = 0; i < cleaned.length(); i++)
        {
            // if (!isalpha(cleaned[i]))
            if (cleaned[i] < 'a' || cleaned[i] > 'z')
            {
                alpha = false;
                break;
            }
        }
        if (!alpha)
            continue;
    }
    printf("Stats read.\n");
    if (maxAllowed == 0 || contextSize == 2)
        printf("\n");
    return 0;
}
