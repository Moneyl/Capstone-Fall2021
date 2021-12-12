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

//Config vars defined in the program. Can be used to determine robot hardware stats
struct VmConfig
{
    std::string Name; //Case insensitive
    VmValue Value;
};

//Program binary that the VM can run
struct VmProgram
{
    VmProgram(const ProgramHeader& header, const std::vector<Instruction>& instructions, const std::vector<VmValue>& variables, const std::vector<VmConfig>& config)
        : Header(header), Instructions(instructions), Variables(variables), Config(config) {}
    VmProgram(const ProgramHeader&& header, const std::vector<Instruction>&& instructions, const std::vector<VmValue>&& variables, const std::vector<VmConfig>&& config)
        : Header(header), Instructions(instructions), Variables(variables), Config(config) {}

    ProgramHeader Header;
    const std::vector<Instruction> Instructions;
    const std::vector<VmValue> Variables;
    const std::vector<VmConfig> Config;

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

        //Write config data
        u32 configCount = Config.size();
        out.write((char*)&configCount, sizeof(u32));
        for (const VmConfig& config : Config)
        {
            //Write value
            out.write((char*)&config.Value, sizeof(VmValue));

            //Write name string
            for (char c : config.Name)
                out.write(&c, 1);

            //Write null terminator at end of name
            char nullTerminator = '\0';
            out.write(&nullTerminator, 1);
        }
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
        std::vector<VmConfig> config = {};

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

        //Read config count
        u32 configCount = 0;
        in.read((char*)&configCount, sizeof(u32));

        //Read config data
        for (u32 i = 0; i < configCount; i++)
        {
            VmConfig& configVal = config.emplace_back();

            //Read value
            in.read((char*)configVal.Value, sizeof(VmValue));

            //Read name as null terminated string
            char c;
            in.read(&c, 1);
            do
            {
                configVal.Name += c;
                in.read(&c, 1);
            } while (c != '\0');
        }

        //Construct and return VmProgram instance
        VmProgram program(std::move(header), std::move(instructions), std::move(variables), std::move(config)); //std::move() used to avoid unecessary copies
        return Success(program);
    }
};