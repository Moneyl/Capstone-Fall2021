#include "Robot.h"
#include "math/Util.h"
#include "render/Renderer.h"
#include "Arena.h"

void Robot::Update(Arena& arena, f32 deltaTime, u32 cyclesToExecute)
{
    if (Error || Health <= 0)
        return;

    //Cycle VM
    for (u32 i = 0; i < cyclesToExecute; i++)
    {
        Result<void, VMError> cycleResult = Vm->Cycle();
        if (cycleResult.Error())
        {
            VMError& error = cycleResult.Error().value();
            printf("Error in VM::Cycle()! Code: %s, Message: %s\n", to_string(error.Code).c_str(), error.Message.c_str());
            Error = true;
            return;
        }

        //Multiplier used to make hardware independent of framerate and cycle rate
        const f32 timePerCycle = deltaTime / (f32)cyclesToExecute;

        //Update robot hardware
        //Movement
        {
            const VmValue& steering = GetPort(Port::Steering);
            Angle += steering * timePerCycle;
            TurretAngle += steering * timePerCycle; //Turret rotates with the chassis

            const VmValue& spedometer = GetPort(Port::Spedometer);
            Speed = spedometer;

            const Vec2<f32> direction = Vec2<f32>(cos(ToRadians(Angle)), sin(ToRadians(Angle))).Normalized();
            Position += direction * Speed * timePerCycle;
        }
        
        //Turret
        {
            VmValue& rotateOffset = GetPort(Port::TurretRotateOffset);
            if (rotateOffset != 0)
            {
                TurretAngle += rotateOffset * timePerCycle;
                rotateOffset = 0;
            }
            
            VmValue& rotateAbsolute = GetPort(Port::TurretRotateAbsolute);
            if (rotateAbsolute != std::numeric_limits<VmValue>::max())
            {
                TurretAngle = rotateAbsolute;
                rotateAbsolute = std::numeric_limits<VmValue>::max(); //Special value for this port to signal that it's not set
            }

            VmValue& shoot = GetPort(Port::TurretShoot);
            if (shoot != 0 && _turretShootTimer >= TurretShootFrequency)
            {
                arena.CreateBullet(Position, TurretDirection(), ID());
                Heat += HeatPerTurretShot;
                _turretShootTimer = 0.0f;
                shoot = 0;
            }
            else
            {
                _turretShootTimer += timePerCycle;
            }
        }

        //Mine layer & detonator
        {
            //Lay mines
            VmValue& mineLayer = GetPort(Port::MineLayer);
            if (mineLayer != 0 && NumMines > 0 && _mineLayerTimer >= MineLayerFrequency)
            {
                arena.CreateMine(Position, ID());
                _mineLayerTimer = 0.0f;
                NumMines--;
            }
            else
            {
                _mineLayerTimer += timePerCycle;
            }

            //Detonate laid mines
            VmValue& mineTrigger = GetPort(Port::MineTrigger);
            if (mineTrigger != 0)
            {
                for (Mine& mine : arena.Mines)
                    if (mine.Creator == ID())
                        arena.DetonateMine(mine);
            }
        }

        //Sonar, radar, and scanner
        {
            //Find closest bot
            Robot* closestBot = arena.GetClosestRobot(Position, this);
            f32 closestBotDistance = closestBot ? (closestBot->Position - Position).Length() : std::numeric_limits<f32>::infinity();

            VmValue& sonar = GetPort(Port::Sonar);
            if (sonar != 0 && _sonarTimer >= RadarSonarFrequency)
            {
                //Todo: Consider implementing the ipo/opo instructions or equivalent so users can determine which register to write the output to
                if (closestBot && closestBotDistance <= RadarSonarRange)
                {
                    //Write heading of nearest bot to r7
                    const Vec2<f32> dir = (closestBot->Position - Position).Normalized();
                    Vm->Registers[7] = dir.AngleUnitDegrees();
                }
                _sonarTimer = 0.0f;
                _sonarOn = true;
            }
            else
            {
                _sonarTimer += timePerCycle;
            }

            VmValue& radar = GetPort(Port::Radar);
            if (radar != 0 && _radarTimer >= RadarSonarFrequency)
            {
                //Write distance of nearest bot to r7
                if (closestBot && closestBotDistance <= RadarSonarRange)
                    Vm->Registers[7] = (closestBot->Position - Position).Length();

                _radarTimer = 0.0f;
                _radarOn = true;
            }
            else
            {
                _radarTimer += timePerCycle;
            }

            VmValue& scanner = GetPort(Port::Scanner);
            if (scanner != 0 && _scannerTimer >= ScannerFrequency)
            {
                //Trigger scanner, write range of nearest target to r7
                Robot* closestBotArc = arena.GetClosestRobot(Position, this, ToRadians(Angle - _scannerArcWidth), ToRadians(Angle + _scannerArcWidth));
                if (closestBotArc)
                    Vm->Registers[7] = (closestBotArc->Position - Position).Length();

                _scannerTimer = 0.0f;
                _scannerOn = true;
            }
            else
            {
                _scannerTimer += timePerCycle;
            }

            VmValue& scannerArc = GetPort(Port::ScannerArc);
            if (scannerArc != 0)
            {
                _scannerArcWidth = std::min((f32)scannerArc, 64.0f);
            }
        }

        //Heatsink
        {
            Heat -= HeatsinkCapacity * timePerCycle;
            Heat = std::max(0.0f, Heat);
            if (Heat >= HeatDamageThreshold)
            {
                Health -= OverHeatDamageFrequency * timePerCycle;
            }
        }
    }
}

void Robot::Draw(Renderer* renderer)
{
    renderer->DrawTriangle(Position, Robot::ChassisSize, Angle, { 0, 127, 0, 255 }); //Chassis
    renderer->DrawLine(Position, Position + (TurretDirection() * Robot::TurretLength), ColorWhite); //Turret
    if (_sonarOn || _radarOn)
        renderer->DrawCircle(Position, RadarSonarRange, ColorWhite); //Sonar/radar arc
    if (_scannerOn)
        renderer->DrawArc(Position, 200.0f, Angle, _scannerArcWidth, ColorWhite, 10); //Scanner arc

    _sonarOn = false;
    _radarOn = false;
    _scannerOn = false;
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

Vec2<f32> Robot::TurretDirection() const
{
    //Todo: Fix whatever is going wrong here. For some reason this isn't calculating the correct turret angle, but the inverse of it.
    //      Short term workaround is to subtract it from 360 to flip the axis
    const f32 turretAngleRadians = ToRadians(360.0f - TurretAngle);
    return Vec2<f32>(cos(turretAngleRadians), sin(turretAngleRadians));
}

std::array<Vec2<f32>, 3> Robot::GetChassisPoints() const
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

bool Robot::Collides(const Vec2<f32>& point) const
{
    const std::array<Vec2<f32>, 3> chassis = GetChassisPoints();
    return IsPositionInTriangle(point, chassis[0], chassis[1], chassis[2]);
}
