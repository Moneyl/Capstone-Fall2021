#include "Robot.h"

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
            VMError& error = cycleResult.ErrorData();
            printf("Error in VM::Cycle()! Code: %s, Message: %s\n", to_string(error.Code).c_str(), error.Message.c_str());
            Error = true;
            return;
        }
    }
}