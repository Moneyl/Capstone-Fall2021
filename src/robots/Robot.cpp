#include "Robot.h"
#include "math/Util.h"

void Robot::Update(f32 deltaTime, u32 cyclesPerFrame)
{
    if (Error)
        return;

    //Cycle VM
    for (u32 i = 0; i < cyclesPerFrame; i++)
    {
        Result<void, VMError> cycleResult = Vm->Cycle();
        if (cycleResult.Error())
        {
            VMError& error = cycleResult.Error().value();
            printf("Error in VM::Cycle()! Code: %s, Message: %s\n", to_string(error.Code).c_str(), error.Message.c_str());
            Error = true;
            return;
        }

        //Update robot hardware
        //Movement
        {
            const VmValue& steering = GetPort(Port::Steering);
            Angle += steering;

            const VmValue& spedometer = GetPort(Port::Spedometer);
            Speed = spedometer;

            const Vec2<f32> direction = Vec2<f32>(cos(ToRadians(Angle)), sin(ToRadians(Angle))).Normalized();
            Position += direction * Speed;
            //Todo: Push back into the arena if outside of it
        }
    }
}

void Robot::LoadProgramFromSource(std::string_view inFilePath)
{
    Vm->LoadProgramFromSource(inFilePath);
    _sourceFileLastWriteTime = std::filesystem::last_write_time(inFilePath);
    _sourceFilePath = inFilePath;
}

void Robot::TryReload()
{
    //Recompile program if source file changed
    if (_sourceFileLastWriteTime != std::filesystem::last_write_time(_sourceFilePath))
    {
        _sourceFileLastWriteTime = std::filesystem::last_write_time(_sourceFilePath);
        std::unique_ptr<VM> newVM = std::unique_ptr<VM>(new VM());
        Result<void, VMError> result = newVM->LoadProgramFromSource(_sourceFilePath);
        std::string sourceFileName = std::filesystem::path(_sourceFilePath).filename().string();

        //In case of error, log it then keep using the already loaded program
        if (result.Error())
        {
            VMError& error = result.Error().value();
            printf("Error reloading '%s'! Error code: %s Error message: %s\n", sourceFileName.c_str(), to_string(error.Code).c_str(), error.Message.c_str());
            return;
        }

        //Successful reload
        Vm = std::move(newVM);
        printf("Recompiled robot '%s'\n", sourceFileName.c_str());
    }
}

VmValue& Robot::GetPort(Port port)
{
    return *(VmValue*)(&Vm->Memory[(VmValue)port]);
}
