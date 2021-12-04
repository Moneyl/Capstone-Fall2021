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

    bool EndsWith(std::string_view str, std::string_view target)
    {
        if (str.length() >= target.length())
            return str.compare(str.length() - target.length(), target.length(), target) == 0; //Check if the last n digits of str equal target, where n = target.length()
        else
            return false; //target.length() > str.length(). Must be false.
    }

    bool IsNumber(std::string_view str)
    {
        //Todo: Either rename the func to ::IsI32() or maybe just have a bunch of specialized instances with types. E.g. String::IsNumber<i32>()
        int num = 0;
        const int base = String::StartsWith(String::ToLower(str), "0x") ? 16 : 10;
        auto res = std::from_chars(str.data(), str.data() + str.length(), num, base);
        return res.ec == std::errc();
    }

    i16 ToShort(std::string_view str)
    {
        //Determine number base
        int num = 0;
        const int base = String::StartsWith(String::ToLower(str), "0x") ? 16 : 10;
        
        //Get substring that std::from_chars can handle
        const char* begin = str.data();
        const char* end = str.data() + str.length();
        if (base == 16)
            begin += 2; //Skip 0x since from_chars doesn't support base prefixes
        
        //Convert string
        auto res = std::from_chars(begin, end, num, base);
        return (i16)num;
    }

    bool Contains(std::string_view str, std::string_view search)
    {
        return str.find(search) != std::string_view::npos;
    }

    bool EqualIgnoreCase(std::string_view str0, std::string_view str1)
    {
        std::string str0Lower = String::ToLower(str0);
        std::string str1Lower = String::ToLower(str1);
        return str0Lower == str1Lower;
    }
}