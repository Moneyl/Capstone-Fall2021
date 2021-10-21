#pragma once
#include "Typedefs.h"
#include "Span.h"
#include <string_view>
#include <vector>
#include <fstream>

namespace File
{
    //Read file to a string
    std::string ReadAll(std::string_view inputFilePath);
    //Read file to a vector of bytes
    std::vector<u8> ReadAllBytes(std::string_view inputFilePath);
    
    //Write string to a file. Overwrites existing contents of the file.
    void WriteAll(std::string_view inputFilePath, std::string_view data);
    //Write bytes to a file
    template<class T>
    void WriteAllBytes(std::string_view inputFilePath, Span<T> data)
    {
        std::ofstream stream(std::string(inputFilePath), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        stream.write((char*)data.data(), data.size_bytes());
    }
}