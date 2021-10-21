#include "File.h"

namespace File
{
    std::string ReadAll(std::string_view inputFilePath)
    {
        //Open file
        std::ifstream file(std::string(inputFilePath), 0);
        if (!file.is_open())
            throw std::runtime_error("Failed to open file \"" + std::string(inputFilePath) + "\"");

        //Seek to the end of the file and get position to determine file size
        file.seekg(0, std::ifstream::_Seekend);
        size_t sizeBytes = file.tellg();

        //Read file to a string
        std::string out;
        file.seekg(0);
        for (u64 i = 0; i < sizeBytes; i++)
        {
            char c;
            file.read(&c, 1);
            out += c;
        }
        file.close();
        return out;
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