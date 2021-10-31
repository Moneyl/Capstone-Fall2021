#pragma once
#include "Typedefs.h"
#include "utility/Result.h"
#include "Instruction.h"
#include <fstream>

//The beginning of any AT Robots program
struct ProgramHeader
{
    u32 Signature; //ASCII "ATRB"
    u32 ProgramSize; //Size of the entire program binary file
    u32 InstructionsSize; //Size of the instruction block
    u32 VariablesSize; //Size of the variable block
};

//Program binary that the VM can run
struct VmProgram
{
    VmProgram(const ProgramHeader& header, const std::vector<Instruction>& instructions, const std::vector<i16>& variables)
        : Header(header), Instructions(instructions), Variables(variables) {}
    VmProgram(const ProgramHeader&& header, const std::vector<Instruction>&& instructions, const std::vector<i16>&& variables)
        : Header(header), Instructions(instructions), Variables(variables) {}

    ProgramHeader Header;
    const std::vector<Instruction> Instructions;
    const std::vector<VmValue> Variables;

    static const u32 EXPECTED_SIGNATURE = ('A' << 0) | ('T' << 8) | ('R' << 16) | ('B' << 24); //ASCII "ATRB"

    //Write to file
    void Write(std::string_view outputFilePath)
    {
        //Open output file. Opened with truncate so all existing data is wiped.
        std::ofstream out(std::string(outputFilePath), std::ofstream::out | std::ofstream::binary | std::ofstream::trunc);
        
        //Write header
        out.write((char*)&Header, sizeof(ProgramHeader));

        //Write instructions
        out.write((char*)Instructions.data(), Instructions.size() * sizeof(Instruction));

        //Write variables
        out.write((char*)Variables.data(), Variables.size() * sizeof(VmValue));
    }

    //Read from file
    static Result<VmProgram, std::string> Read(std::string_view inputFilePath)
    {
        //Open file
        std::ifstream in(std::string(inputFilePath), std::ifstream::in | std::ifstream::binary);

        //Create storage for each data block
        ProgramHeader header;
        std::vector<Instruction> instructions = {};
        std::vector<VmValue> variables = {};

        //Read header
        in.read((char*)&header, sizeof(ProgramHeader));
        
        //Validate header
        if (header.Signature != VmProgram::EXPECTED_SIGNATURE)
            return Error("Error loading VM program from a file. Invalid header signature. Expected " + std::to_string(VmProgram::EXPECTED_SIGNATURE) + ", detected " + std::to_string(header.Signature));

        //Reserve enough space for each data block
        instructions.resize(header.InstructionsSize / sizeof(Instruction));
        variables.resize(header.VariablesSize / sizeof(VmValue));

        //Read instructions
        in.read((char*)instructions.data(), header.InstructionsSize);

        //Read variables
        in.read((char*)variables.data(), header.VariablesSize);

        //Construct and return VmProgram instance
        VmProgram program(std::move(header), std::move(instructions), std::move(variables)); //std::move() used to avoid unecessary copies
        return Success(program);
    }
};