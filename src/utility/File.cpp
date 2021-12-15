#include "File.h"
#include <sstream>

namespace File
{
    std::string ReadAll(std::string_view inputFilePath)
    {
        //Open file
        std::ifstream file(std::string(inputFilePath), 0);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file \"" + std::string(inputFilePath) + "\"");

        //Read file to string
        std::stringstream sstream;
        sstream << file.rdbuf();
        return sstream.str();
    }

    std::vector<u8> ReadAllBytes(std::string_view inputFilePath)
    {
        //Open file
        std::ifstream file(std::string(inputFilePath), std::ios::binary);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file \"" + std::string(inputFilePath) + "\"");

        //Seek to the end of the file and get position to determine file size
        file.seekg(0, std::ifstream::_Seekend);
        size_t sizeBytes = file.tellg();

        //Create a vector for the bytes and reserve enough space for them
        std::vector<u8> bytes;
        bytes.reserve(sizeBytes);

        //Read bytes to vector
        file.seekg(0);
        file.read((char*)bytes.data(), sizeBytes);
        file.close();
        return bytes;
    }
    
    void WriteAll(std::string_view inputFilePath, std::string_view data)
    {
        std::ofstream stream(std::string(inputFilePath), std::ofstream::out | std::ofstream::trunc);
        stream.write(data.data(), data.size());
    }
}