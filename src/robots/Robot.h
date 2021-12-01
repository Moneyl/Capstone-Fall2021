#pragma once
#include "Typedefs.h"
#include "vm/VM.h"
#include "math/Vec2.h"
#include "vm/Constants.h"
#include <filesystem>
#include <memory> //For std::unique_ptr<T>

class Renderer;
class Arena;

//Robot tank used in the arena. Each has a single VM running a program that controls the robots hardware.
class Robot
{
public:
    Robot()
    {
        static u64 id = 0;
        _id = id++; //Give each robot a unique ID
    }

    //Per frame update
    void Update(Arena& arena, f32 deltaTime, u32 cyclesPerFrame);
    //Draw robot in the window
    void Draw(Renderer* renderer);
    //Load program from asm source file
    void LoadProgramFromSource(std::string_view inFilePath);
    //Recompile program if the source file was edited since last reload
    void TryReload();
    //Get reference to a VM port
    VmValue& GetPort(Port port);
    //Path of source file
    const std::string& SourcePath() const { return _sourceFilePath; }
    //Use to uniquely identify a robot
    u64 ID() { return _id; }
    Vec2<f32> TurretDirection();

    //Virtual machine that runs the robots logic program
    std::unique_ptr<VM> Vm = std::unique_ptr<VM>(new VM());
    Vec2<VmValue> Position = { 0, 0 }; //Robot position in arena
    VmValue Angle = 0; //Robot chassis angle in degress
    VmValue Speed = 0; //Current speed
    VmValue TurretAngle = 0;

    //Set to true when an error occurs. If true ::Update() is stopped until the error is resolved.
    bool Error = false;

    static const inline f32 TurretLength = 12.0f;
    static const inline f32 ChassisSize = 10.0f;
private:
    std::filesystem::file_time_type _sourceFileLastWriteTime;
    std::string _sourceFilePath;
    u64 _id;
};