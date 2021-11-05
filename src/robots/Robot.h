#pragma once
#include "Typedefs.h"
#include "vm/VM.h"
#include "math/Vec2.h"
#include <memory> //For std::unique_ptr<T>

//Robot tank used in the arena. Each has a single VM running a program that controls the robots hardware.
class Robot
{
public:
    void Update(f32 deltaTime, u32 cyclesPerFrame);

    //Virtual machine that runs the robots logic program
    std::unique_ptr<VM> Vm = std::unique_ptr<VM>(new VM());

    Vec2<VmValue> Position = { 0, 0 }; //Robot position in arena
    VmValue Angle = 0; //Robot chassis angle in degress

    //Set to true when an error occurs. If true ::Update() is stopped until the error is resolved.
    bool Error = false;
private:
};