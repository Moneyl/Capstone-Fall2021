#include "Robot.h"
#include "math/Util.h"
#include "render/Renderer.h"
#include "Arena.h"

void Robot::Update(Arena& arena, f32 deltaTime, u32 cyclesPerFrame)
{
    if (Error || Health <= 0)
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

        //Todo: Make hardware framerate independent (e.g. framerate shouldn't affect movement speed or turn rate)
        //Todo: Also make it cycle/s independent. Currently increases cycles/s results in faster movement
        //Update robot hardware
        //Movement
        {
            const VmValue& steering = GetPort(Port::Steering);
            Angle += steering;
            TurretAngle += steering; //Turret rotates with the chassis

            const VmValue& spedometer = GetPort(Port::Spedometer);
            Speed = spedometer;

            const Vec2<f32> direction = Vec2<f32>(cos(ToRadians(Angle)), sin(ToRadians(Angle))).Normalized();
            Position += direction * Speed;
        }
        
        //Turret
        {
            VmValue& rotateOffset = GetPort(Port::TurretRotateOffset);
            if (rotateOffset != 0)
            {
                TurretAngle += rotateOffset;
                rotateOffset = 0;
            }
            
            VmValue& rotateAbsolute = GetPort(Port::TurretRotateAbsolute);
            if (rotateAbsolute != std::numeric_limits<VmValue>::max())
            {
                TurretAngle = rotateAbsolute;
                rotateAbsolute = std::numeric_limits<VmValue>::max(); //Special value for this port to signal that it's not set
            }

            VmValue& shoot = GetPort(Port::TurretShoot);
            if (shoot != 0)
            {
                arena.CreateBullet(Position, TurretDirection(), ID());
                shoot = 0;
            }
        }
    }
}

void Robot::Draw(Renderer* renderer)
{
    //Chassis
    renderer->DrawTriangle(Position, Robot::ChassisSize, Angle, { 0, 127, 0, 255 });

    //Turret
    renderer->DrawLine(Position, Position + TurretDirection() * Robot::TurretLength, ColorWhite);
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

Vec2<f32> Robot::TurretDirection()
{
    const f32 turretAngleRadians = ToRadians(TurretAngle);
    return Vec2<f32>(cos(turretAngleRadians), sin(turretAngleRadians)).Normalized();
}

std::array<Vec2<f32>, 3> Robot::GetChassisPoints()
{
    return
    {
        Vec2<f32>{Position + Vec2<f32>{ ChassisSize, 0 }},
        Vec2<f32>{Position + Vec2<f32>{ -ChassisSize, ChassisSize * 0.75f }},
        Vec2<f32>{Position + Vec2<f32>{ -ChassisSize, -ChassisSize * 0.75f }}
    };
}

void Robot::Damage(VmValue damage)
{
    if (Health > 0)
        Health -= damage;
}
