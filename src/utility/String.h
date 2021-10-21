#pragma once
#include "Typedefs.h"
#include <string_view>
#include <vector>

namespace String
{
    //Split a string at each instance of the delimiter. The original string must be alive while the returned substrings are used.
    std::vector<std::string_view> Split(std::string_view str, std::string_view delimiter);

    //Return a lowercase copy of the string
    std::string ToLower(std::string_view str);

    //Returns true if the string starts with the target string
    bool StartsWith(std::string_view str, std::string_view target);

    //Returns true if the string is a valid number. Supports hex strings by default.
    bool IsNumber(std::string_view str, bool allowHex = true);

    //Converts string to a 16bit signed integer. Supports hex by prefixing with 0x
    i16 ToShort(std::string_view str, bool allowHex = true);
}