#include "Robot.h"
#include "math/Util.h"
#include "render/Renderer.h"
#include "Arena.h"

Robot::Robot()
{
    //Give each robot a unique ID
    static u64 id = 0;
    _id = id++;

    //Bind ports callbacks to VM
    Vm->OnPortRead = std::bind(&Robot::OnPortRead, this, std::placeholders::_1, std::placeholders::_2);
    Vm->OnPortWrite = std::bind(&Robot::OnPortWrite, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void Robot::Update(Arena& arena, f32 deltaTime, u32 cyclesToExecute)
{
    _arena = &arena;
    if (Error || Health <= 0)
        return;

    //Multiplier used to make hardware independent of framerate and cycle rate
    const f32 timePerCycle = deltaTime / (f32)cyclesToExecute;

    //Cycle VM
    for (u32 i = 0; i < cyclesToExecute; i++)
    {
        Result<void, VMError> cycleResult = Vm->Cycle(timePerCycle);
        if (cycleResult.Error())
        {
            VMError& error = cycleResult.Error().value();
            printf("Error in VM::Cycle()! Code: %s, Message: %s\n", to_string(error.Code).c_str(), error.Message.c_str());
            Error = true;
            return;
        }

        //Update hardware timers
        _turretShootTimer += timePerCycle;
        _mineLayerTimer += timePerCycle;
        _sonarTimer += timePerCycle;
        _radarTimer += timePerCycle;
        _scannerTimer += timePerCycle;

        //Movement
        const Vec2<f32> direction = Vec2<f32>(cos(ToRadians(Angle)), sin(ToRadians(Angle))).Normalized();
        LastPosition = Position;
        Position += direction * Speed * timePerCycle;

        //Heatsink
        Heat -= HeatsinkCapacity * timePerCycle;
        Heat = std::max(0.0f, Heat);
        if (Heat >= HeatDamageThreshold)
            Health -= OverHeatDamageFrequency * timePerCycle;
    }
}

void Robot::OnPortRead(Port port, f32 deltaTime)
{
    //VM executed ipo. Write hardware state to port if applicable
    switch (port)
    {
        case Port::Spedometer:
            break;
        case Port::Heat:
            break;
        case Port::Compass:
            break;
        case Port::Steering:
            break;
        case Port::TurretShoot:
            break;
        case Port::TurretRotateOffset:
            break;
        case Port::TurretRotateAbsolute:
            break;
        case Port::MineLayer:
            break;
        case Port::MineTrigger:
            break;
        case Port::Sonar: //Is either 0 or the result of the last `opo P_SONAR`
            break;
        case Port::Radar: //Is either 0 or the result of the last `opo P_RADAR`
            break;
        case Port::Scanner:
            break;
        case Port::ScannerArc:
            break;
        default:
            break;
    }
}

void Robot::OnPortWrite(Port port, VmValue value, f32 deltaTime)
{
    //VM executed opo. Update hardware connected to the port
    //Ports aren't required to be set to the provided value. It can be discarded after use.
    switch (port)
    {
    case Port::Spedometer:
        Speed = value;
        break;
    case Port::Heat:
        break;
    case Port::Compass:
        break;
    case Port::Steering:
        Angle += value * deltaTime;
        TurretAngle += value * deltaTime; //Turret rotates with the chassis
        break;
    case Port::TurretShoot:
        if (_turretShootTimer >= TurretShootFrequency)
        {
            _arena->CreateBullet(Position, TurretDirection(), ID());
            Heat += HeatPerTurretShot;
            _turretShootTimer = 0.0f;
        }
        break;
    case Port::TurretRotateOffset:
        TurretAngle += (f32)value * deltaTime; //Rotate turret by value stored in port
        break;
    case Port::TurretRotateAbsolute:
        TurretAngle = (f32)value; //Set turret angle to the value stored in port
        break;
    case Port::MineLayer:
        if (NumMines > 0 && _mineLayerTimer >= MineLayerFrequency)
        {
            _arena->CreateMine(Position, ID());
            _mineLayerTimer = 0.0f;
            NumMines--;
        }
        break;
    case Port::MineTrigger:
        //Detonate laid mines
        for (Mine& mine : _arena->Mines)
            if (mine.Creator == ID())
                _arena->DetonateMine(mine);
        break;
    case Port::Sonar:
        if (_sonarTimer >= RadarSonarFrequency)
        {
            //Sonar triggered
            _sonarTimer = 0.0f;
            _sonarOn = true;

            //Check for robot in sonar range
            Robot* closestBot = _arena->GetClosestRobot(Position, this);
            f32 closestBotDistance = closestBot ? (closestBot->Position - Position).Length() : std::numeric_limits<f32>::infinity();
            if (closestBot && closestBotDistance <= RadarSonarRange)
            {
                //Write heading of nearest bot back to the port
                const Vec2<f32> dir = (closestBot->Position - Position).Normalized();
                Vm->GetPort(Port::Sonar) = (VmValue)dir.AngleUnitDegrees();
            }
        }
        break;
    case Port::Radar:
        if (_radarTimer >= RadarSonarFrequency)
        {
            //Radar triggered
            _radarTimer = 0.0f;
            _radarOn = true;

            //Check for robot in radar range
            Robot* closestBot = _arena->GetClosestRobot(Position, this);
            f32 closestBotDistance = closestBot ? (closestBot->Position - Position).Length() : std::numeric_limits<f32>::infinity();
            if (closestBot && closestBotDistance <= RadarSonarRange)
            {
                //Write distance of nearest bot back to the port
                Vm->GetPort(Port::Radar) = (closestBot->Position - Position).Length();
            }
        }
        break;
    case Port::Scanner:
        if (_scannerTimer >= ScannerFrequency)
        {
            //Scanner triggered
            _scannerTimer = 0.0f;
            _scannerOn = true;

            //Check for robot in scanner arc
            Robot* closestBotArc = _arena->GetClosestRobot(Position, this, ToRadians(Angle - _scannerArcWidth), ToRadians(Angle + _scannerArcWidth));
            if (closestBotArc)
            {
                //Write distance to robot back into the port
                Vm->GetPort(Port::Scanner) = (closestBotArc->Position - Position).Length();
            }
        }
        break;
    case Port::ScannerArc:
        _scannerArcWidth = Range((f32)value, 0.0f, 64.0f);
        break;
    default:
        break;
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

        //Bind ports callbacks to new VM
        Vm->OnPortRead = std::bind(&Robot::OnPortRead, this, std::placeholders::_1, std::placeholders::_2);
        Vm->OnPortWrite = std::bind(&Robot::OnPortWrite, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        printf("Recompiled robot '%s'\n", sourceFileName.c_str());
    }
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

bool Robot::PointInChassis(const Vec2<f32>& point) const
{
    const std::array<Vec2<f32>, 3> chassis = GetChassisPoints();
    return IsPositionInTriangle(point, chassis[0], chassis[1], chassis[2]);
}

bool Robot::ChassisInRectangle(const Vec2<f32>& rectPos, const Vec2<f32>& rectSize) const
{
    const std::array<Vec2<f32>, 3> chassis = GetChassisPoints();
    return IsPositionInRect(chassis[0], rectPos, rectSize) &&
           IsPositionInRect(chassis[1], rectPos, rectSize) &&
           IsPositionInRect(chassis[2], rectPos, rectSize);
}