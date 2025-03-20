#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <map>
#include <random>

#define DEFAULT_CONTEXT 3
#define END_CHANCE_RATIO_PERCENTAGE 0.7
#define MAX_GENERATION_TRIES 500

int randint(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
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

std::string utf8_char_at(const std::string &str, size_t n, std::string defaultVal)
{
    std::string ret = utf8_char_at(str, n);
    if (ret.empty())
        return defaultVal;
    return ret;
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
class WordModel
{
private:
    std::vector<size_t> lengthsFrequencies;
    std::vector<std::map<std::string, std::map<std::string, size_t>>> maps;
    unsigned int contextSize;
    size_t totalWordsLearned;

public:
    WordModel(int contextSize);
    ~WordModel();
    void addStr(std::string str, std::string c);
    void addLength(int length);
    std::string aggregateWordGen(std::string begin);
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
        maps.at(sizeOfStr).insert(std::make_pair(ctx, std::map<std::string, size_t>()));
        maps.at(sizeOfStr)[ctx].insert(std::make_pair(c, 1));
    }
}

void WordModel::addLength(int length)
{
    totalWordsLearned++;
    for (int i = lengthsFrequencies.size(); i < length; i++)
    {
        lengthsFrequencies.emplace_back(0);
    }

    lengthsFrequencies.at(length - 1)++;
}

std::string WordModel::aggregateWordGen(std::string begin)
{
    size_t sizeOfStr = utf8_length(begin);
    std::string ctxSearch;
    int ctxSize = contextSize - 1;
    if (sizeOfStr < contextSize)
    {
        ctxSearch = " " + begin;
        ctxSize = sizeOfStr;
    }
    else
    {
        int beginIndex = std::max(0, (int)sizeOfStr - (int)contextSize);
        ctxSearch = (""); // begin.substr(beginIndex, contextSize);
        for (size_t i = 0; i < contextSize; i++)
        {
            ctxSearch += utf8_char_at(begin, beginIndex + i);
        }
    }
    if (!maps.at(ctxSize).count(ctxSearch) || maps.at(ctxSize)[ctxSearch].empty())
        return begin + "\n";

    size_t sum(0), numberOfEOL(0);
    for (auto it = maps.at(ctxSize)[ctxSearch].begin(); it != maps.at(ctxSize)[ctxSearch].end(); ++it)
    {
        // std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
        if (it->first.compare("\n") == 0)
            numberOfEOL += it->second;
        else
            sum += it->second;
    }

    size_t numberOfBiggerWords(0);
    for (size_t indSumIndex = sizeOfStr; indSumIndex < lengthsFrequencies.size(); indSumIndex++)
    {
        numberOfBiggerWords += lengthsFrequencies[indSumIndex];
    }
    float ratioPhase = END_CHANCE_RATIO_PERCENTAGE * (totalWordsLearned - numberOfBiggerWords) / totalWordsLearned;
    float EOLMultiplierFactor = numberOfEOL == 0 ? 0 : (ratioPhase * sum) / ((1 - ratioPhase) * numberOfEOL);
    sum += EOLMultiplierFactor * numberOfEOL;

    if (sum == 0)
        return begin + "\n";

    size_t indexCharChosen = randint(0, sum - 1);
    for (auto it = maps.at(ctxSize)[ctxSearch].begin(); it != maps.at(ctxSize)[ctxSearch].end(); ++it)
    {
        // std::string current = it->first;
        if (it->first.compare("\n") == 0)
        {
            if (indexCharChosen < it->second * EOLMultiplierFactor)
                return begin + it->first;
            indexCharChosen -= it->second * EOLMultiplierFactor;
        }
        else
        {
            // std::cout << "Key: " << it->first << ", Value: " << it->second << std::endl;
            if (indexCharChosen < it->second)
                return begin + it->first;
            indexCharChosen -= it->second;
        }
    }

    return begin + "\n";
}

bool wordIn(const std::string &word, const std::vector<std::string> &list)
{
    for (size_t i = 0; i < list.size(); i++)
    {
        if (list.at(i).compare(word) == 0)
            return true;
    }
    return false;
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

    WordModel model(contextSize);

    std::ifstream infile(argv[1]);
    std::ofstream outfile(argv[2]);

    std::string line;

    long unsigned int generatedNumber = atoi(argv[3]);

    unsigned int lines(0);
    printf("Reading source file...\nLine n°0");
    fflush(stdout);
    while (std::getline(infile, line))
    {
        lines++;
        printf("\rLine n°%d", lines);
        fflush(stdout);
        // printf("Str=%s : len=%ld, size=%ld, utf8=%ld | ", line.c_str(), line.length(), line.size(), utf8_length(line));
        std::transform(line.begin(), line.end(), line.begin(), ::tolower);
        // printf("len=%ld, size=%ld, utf8=%ld >>> ", line.length(), line.size(), utf8_length(line));
        std::string cleaned = deleteChar(line, 13) + "\n";
        // printf("len=%ld, size=%ld, utf8=%ld\n", cleaned.length(), cleaned.size(), utf8_length(cleaned));
        size_t length = utf8_length(cleaned);
        model.addLength(length);
        for (size_t lastc = 0; lastc < length; lastc++)
        {
            std::string ctx("");
            for (size_t i = std::max(0, (int)lastc - contextSize); i < lastc && lastc > 0; i++)
            {
                ctx += utf8_char_at(line, i);
            }
            std::string charToPut = utf8_char_at(line, lastc, "\n");
            model.addStr(ctx, charToPut);
        }
    }
    printf("\rStats generated.\nStart generating words...\n###########################################\n");
    std::vector<std::string> foundWords(0);
    int maxTries = 0;
    for (size_t i = 0; i < generatedNumber; i++)
    {
        std::string newWord = "";
        while (newWord.empty() || newWord.back() != '\n')
        {
            newWord = model.aggregateWordGen(newWord);
        }
        if (wordIn(newWord, foundWords))
        {
            maxTries++;
            if (maxTries < MAX_GENERATION_TRIES)
                i--;
        }
        else
        {
            foundWords.emplace_back(newWord);
            std::cout << newWord;
            // outfile << newWord;
            // std::cout << "size: " << utf8_length(newWord) << ", " << newWord;
        }
    }

    return 0;
}
