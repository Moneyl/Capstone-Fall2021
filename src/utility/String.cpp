#include "String.h"
#include <algorithm>
#include <charconv>

namespace String
{
    std::vector<std::string_view> Split(std::string_view str, std::string_view delimiter)
    {
        //Repeatedly split the string by delimiter until the end is reached
        std::vector<std::string_view> output;
        std::string_view::const_iterator first = str.begin();
        while (first != str.end())
        {
            //Split string by next instance of the delimiter
            std::string_view::const_iterator second = std::find_first_of(first, std::cend(str), std::cbegin(delimiter), std::cend(delimiter));
            if (first != second)
                output.emplace_back(str.substr(std::distance(str.begin(), first), std::distance(first, second)));

            //Delimiter not found, no more substrings
            if (second == str.end())
                break;

            first = std::next(second);
        }

        return output;
    }

    std::string ToLower(std::string_view str)
    {
        std::string lower;
        for (char c : str)
            if (c >= 'A' && c <= 'Z') //Convert uppercase characters to lowercase
                lower += c + ('a' - 'A');
            else
                lower += c;

        return lower;
    }

    bool StartsWith(std::string_view str, std::string_view target)
    {
        return str.compare(0, target.length(), target) == 0; //True if target string is at index 0
    }

    bool IsNumber(std::string_view str, bool allowHex)
    {
        //Todo: Either rename the func to ::IsI32() or maybe just have a bunch of specialized instances with types. E.g. String::IsNumber<i32>()
        int num = 0;
        const int base = String::StartsWith(String::ToLower(str), "0x") ? 16 : 10;
        auto res = std::from_chars(str.data(), str.data() + str.length(), num, base);
        return res.ec == std::errc();
    }

    i16 ToShort(std::string_view str, bool allowHex)
    {
        int num = 0;
        const int base = String::StartsWith(String::ToLower(str), "0x") ? 16 : 10;
        auto res = std::from_chars(str.data(), str.data() + str.length(), num, base);
        return (i16)num;
    }
}