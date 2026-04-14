#pragma once

#include <string>
#include <vector>

namespace StringUtil
{
    static void SplitAndTrim(const std::string& str, char tokenizer, std::vector<std::string>& result)
    {
        result.clear();

        size_t beg = 0;
        while (beg <= str.length())
        {
            size_t off = str.find(tokenizer, beg);
            if (off == std::string::npos)
                off = str.length();

            std::string token = str.substr(beg, off - beg);

            // safe trim
            auto first = token.find_first_not_of(" \t\n\r\f\v");
            if (first == std::string::npos)
            {
                token.clear();
            }
            else
            {
                auto last = token.find_last_not_of(" \t\n\r\f\v");
                token = token.substr(first, last - first + 1);
            }

            result.push_back(token);
            beg = off + 1;
        }
    }
}