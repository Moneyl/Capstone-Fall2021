#include "Robot.h"
#include "math/Util.h"
#include "render/Renderer.h"
#include "Arena.h"

Robot::Robot()
{
    //Give each robot a unique ID
    static u64 id = 0;
    _id = id++;
    Init();
}

void Robot::Update(Arena& arena, f32 deltaTime, u32 cyclesToExecute)
{
    _arena = &arena;
    if (Error || Armor <= 0)
        return;

    //Multiplier used to make hardware independent of framerate and cycle rate
    const f32 timePerCycle = deltaTime / (f32)cyclesToExecute;
    LastPosition = Position;

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
        Position += direction * Speed * timePerCycle;

        //Heatsink
        Heat -= Heatsink * timePerCycle;
        Heat = std::max(0.0f, Heat);
        if (Heat >= HeatDamageThreshold)
            Armor -= OverHeatDamageFrequency * timePerCycle;
    }
}

void Robot::OnPortRead(Port port, f32 deltaTime)
{
    //VM executed ipo. Write hardware state to port if applicable
    //Ports that do nothing in this function are write only
    switch (port)
    {
        case Port::Spedometer:
            Vm->GetPort(Port::Spedometer) = Speed;
            break;
        case Port::Steering:
            break;
        case Port::TurretShoot:
            break;
        case Port::TurretRotateOffset:
            Vm->GetPort(Port::TurretRotateOffset) = TurretAngle - Angle;
            break;
        case Port::TurretRotateAbsolute:
            Vm->GetPort(Port::TurretRotateAbsolute) = TurretAngle;
            break;
        case Port::MineLayer:
            Vm->GetPort(Port::MineLayer) = NumMines;
            break;
        case Port::MineTrigger:
            Vm->GetPort(Port::MineTrigger) = MaxMines - NumMines; //# of mines laid
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
                Robot* closestBotArc = _arena->GetClosestRobot(Position, this, ToRadians(TurretAngle - _scannerArcWidth / 2.0f), ToRadians(TurretAngle + _scannerArcWidth / 2.0f));
                if (closestBotArc)
                {
                    //Write distance to robot back into the port
                    Vm->GetPort(Port::Scanner) = (closestBotArc->Position - Position).Length();

                    //Calculate accuracy
                    f32 angleToBot = (closestBotArc->Position - Position).AngleUnitDegrees();
                    Accuracy = angleToBot - TurretAngle;
                }
            }
            break;
        case Port::ScannerArc:
            Vm->GetPort(Port::ScannerArc) = _scannerArcWidth;
            break;
        case Port::Throttle:
            break;
        case Port::Heat:
            Vm->GetPort(Port::Heat) = Heat;
            break;
        case Port::Compass:
            Vm->GetPort(Port::Compass) = Angle;
            break;
        case Port::Armor:
            Vm->GetPort(Port::Armor) = Armor;
            break;
        case Port::Random:
        {
            int randVal = rand() - RAND_MAX / 2; //Generate random value and subtract RAND_MAX / 2 to allow negative numbers
            randVal %= std::numeric_limits<VmValue>::max(); //Scale to range of VmValue
            Vm->GetPort(Port::Random) = randVal;
        }
            break;
        case Port::Shield:
            Vm->GetPort(Port::Shield) = ShieldOn;
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
        break;
    case Port::Steering:
        Angle += value * deltaTime;
        TurretAngle += value * deltaTime; //Turret rotates with the chassis
        break;
    case Port::TurretShoot:
        if (_turretShootTimer >= TurretShootFrequency)
        {
            //Calculate shoot angle. Can be slightly adjusted by writing a non zero value to the port
            f32 shootAdjustment = Range(Vm->GetPort(Port::TurretShoot), -TurretShootAngleControl, TurretShootAngleControl);
            f32 shootAngle = ToRadians(360.0f - TurretDirection().AngleUnitDegrees() + shootAdjustment);
            Vec2<f32> shootDirection = { cos(shootAngle), sin(shootAngle) };

            //Shoot the turret
            _arena->CreateBullet(Position, shootDirection, ID(), _turretDamage);
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
            _arena->CreateMine(Position, ID(), MineDamage);
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
        break;
    case Port::Radar:
        break;
    case Port::Scanner:
        break;
    case Port::ScannerArc:
        _scannerArcWidth = Range((f32)value, 0.0f, 64.0f);
        break;
    case Port::Throttle:
    {
        f32 throttleNormalized = Range((f32)value / 100.0f, -1.0f, 1.0f); //Adjust port value range from [-100, 100] to [0.0f, 1.0f]
        f32 newSpeed = throttleNormalized * MaxSpeed;
        Speed = newSpeed;
    }
        break;
    case Port::Heat:
        break;
    case Port::Compass:
        break;
    case Port::Armor:
        break;
    case Port::Random:
        break;
    case Port::Shield:
        ShieldOn = Vm->GetPort(Port::Shield) != 0;
        break;
    default:
        break;
    }
}

void Robot::Init()
{
    //Bind ports callbacks to VM
    Vm->OnPortRead = std::bind(&Robot::OnPortRead, this, std::placeholders::_1, std::placeholders::_2);
    Vm->OnPortWrite = std::bind(&Robot::OnPortWrite, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    //Read config values
    std::optional<VmValue> scanner = Vm->GetConfigOr("scanner");
    std::optional<VmValue> turret = Vm->GetConfigOr("turret");
    std::optional<VmValue> armor = Vm->GetConfigOr("armor");
    std::optional<VmValue> engine = Vm->GetConfigOr("engine");
    std::optional<VmValue> heatsink = Vm->GetConfigOr("heatsink");
    std::optional<VmValue> mines = Vm->GetConfigOr("mines");
    std::optional<VmValue> shield = Vm->GetConfigOr("shield");

    //Track missing values to automatically distribute unused points between
    std::vector<std::string> missingConfigValues = {};

    //Setup hardware based on config values. Maximum of Robot::MaxConfigPoints can be spread between them.
    VmValue pointsTotal = 0;
    if (scanner && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(scanner.value(), pointsRemaining);
        pointsTotal += points;
        SetupScanner(points);
    }
    else
        missingConfigValues.push_back("scanner");

    if (turret && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(turret.value(), pointsRemaining);
        pointsTotal += points;
        SetupTurret(points);
    }
    else
        missingConfigValues.push_back("turret");

    if (armor && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(armor.value(), pointsRemaining);
        pointsTotal += points;
        SetupArmor(points);
    }
    else
        missingConfigValues.push_back("armor");

    if (engine && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(engine.value(), pointsRemaining);
        pointsTotal += points;
        SetupEngine(points);
    }
    else
        missingConfigValues.push_back("engine");

    if (heatsink && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(heatsink.value(), pointsRemaining);
        pointsTotal += points;
        SetupHeatsink(points);
    }
    else
        missingConfigValues.push_back("heatsink");

    if (mines && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(mines.value(), pointsRemaining);
        pointsTotal += points;
        SetupMines(points);
    }
    else
        missingConfigValues.push_back("mines");

    if (shield && pointsTotal < MaxConfigPoints)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        const VmValue points = std::min(shield.value(), pointsRemaining);
        pointsTotal += points;
        SetupShield(points);
    }
    else
        missingConfigValues.push_back("shield");

    //Distribute remaining points between any missing config values
    for (const std::string& value : missingConfigValues)
    {
        const VmValue pointsRemaining = MaxConfigPoints - pointsTotal;
        if (pointsRemaining <= 0)
            break; //Out of points

        //Apply default point counts to hardware
        if (value == "scanner")
        {
            const VmValue points = std::min((VmValue)5, pointsRemaining);
            pointsTotal += points;
            SetupScanner(points);
        }
        if (value == "turret")
        {
            const VmValue points = std::min((VmValue)2, pointsRemaining);
            pointsTotal += points;
            SetupTurret(points);
        }
        if (value == "armor")
        {
            const VmValue points = std::min((VmValue)2, pointsRemaining);
            pointsTotal += points;
            SetupArmor(points);
        }
        if (value == "engine")
        {
            const VmValue points = std::min((VmValue)2, pointsRemaining);
            pointsTotal += points;
            SetupEngine(points);
        }
        if (value == "heatsink")
        {
            const VmValue points = std::min((VmValue)1, pointsRemaining);
            pointsTotal += points;
            SetupHeatsink(points);
        }
        if (value == "mines")
        {
            const VmValue points = std::min((VmValue)0, pointsRemaining);
            pointsTotal += points;
            SetupMines(points);
        }
        if (value == "shield")
        {
            const VmValue points = std::min((VmValue)0, pointsRemaining);
            pointsTotal += points;
            SetupShield(points);
        }
    }
}

void Robot::SetupScanner(VmValue points)
{
    switch (points)
    {
    case 0:
        _scannerRange = 250.0f;
        break;
    case 1:
        _scannerRange = 350.0f;
        break;
    case 2:
        _scannerRange = 500.0f;
        break;
    case 3:
        _scannerRange = 700.0f;
        break;
    case 4:
        _scannerRange = 1000.0f;
        break;
    case 5:
        _scannerRange = 1500.0f;
        break;
    default:
        printf("Out of range config value for 'scanner' of %d\n", points);
        break;
    }
}

void Robot::SetupTurret(VmValue points)
{
    switch (points)
    {
    case 0:
        _turretDamage = 0.5f;
        break;
    case 1:
        _turretDamage = 0.8f;
        break;
    case 2:
        _turretDamage = 1.0f;
        break;
    case 3:
        _turretDamage = 1.2f;
        break;
    case 4:
        _turretDamage = 1.35f;
        break;
    case 5:
        _turretDamage = 1.5f;
        break;
    default:
        printf("Out of range config value for 'turret' of %d\n", points);
        break;
    }
}

void Robot::SetupArmor(VmValue points)
{
    switch (points)
    {
    case 0:
        MaxArmor = Armor = ArmorBase * 0.5f;
        ArmorDamageMultiplier = 1.33f;
        break;
    case 1:
        MaxArmor = Armor = ArmorBase * 0.66f;
        ArmorDamageMultiplier = 1.2f;
        break;
    case 2:
        MaxArmor = Armor = ArmorBase * 1.0f;
        ArmorDamageMultiplier = 1.0f;
        break;
    case 3:
        MaxArmor = Armor = ArmorBase * 1.2f;
        ArmorDamageMultiplier = 0.85f;
        break;
    case 4:
        MaxArmor = Armor = ArmorBase * 1.3f;
        ArmorDamageMultiplier = 0.75f;
        break;
    case 5:
        MaxArmor = Armor = ArmorBase * 1.5f;
        ArmorDamageMultiplier = 0.66f;
        break;
    default:
        printf("Out of range config value for 'armor' of %d\n", points);
        break;
    }
}

void Robot::SetupEngine(VmValue points)
{
    switch (points)
    {
    case 0:
        Engine = 0.5f;
        break;
    case 1:
        Engine = 0.8f;
        break;
    case 2:
        Engine = 1.0f;
        break;
    case 3:
        Engine = 1.2f;
        break;
    case 4:
        Engine = 1.35f;
        break;
    case 5:
        Engine = 1.5f;
        break;
    default:
        printf("Out of range config value for 'engine' of %d\n", points);
        break;
    }
    MaxSpeed = SpeedBase * Engine;
}

void Robot::SetupHeatsink(VmValue points)
{
    switch (points)
    {
    case 0:
        Heatsink = 0.75f;
        break;
    case 1:
        Heatsink = 1.0f;
        break;
    case 2:
        Heatsink = 1.125f;
        break;
    case 3:
        Heatsink = 1.25f;
        break;
    case 4:
        Heatsink = 1.33f;
        break;
    case 5:
        Heatsink = 1.5f;
        break;
    default:
        printf("Out of range config value for 'heatsink' of %d\n", points);
        break;
    }
}

void Robot::SetupMines(VmValue points)
{
    switch (points)
    {
    case 0:
        NumMines = 2;
        break;
    case 1:
        NumMines = 4;
        break;
    case 2:
        NumMines = 6;
        break;
    case 3:
        NumMines = 10;
        break;
    case 4:
        NumMines = 16;
        break;
    case 5:
        NumMines = 24;
        break;
    default:
        printf("Out of range config value for 'mines' of %d\n", points);
        break;
    }
}

void Robot::SetupShield(VmValue points)
{
    switch (points)
    {
    case 0:
    case 1:
    case 2:
        Shields = 0.0f;
        break;
    case 3:
        Shields = 2.0f / 3.0f;
        break;
    case 4:
        Shields = 1.0f / 2.0f;
        break;
    case 5:
        Shields = 1.0f / 3.0f;
        break;
    default:
        printf("Out of range config value for 'shield' of %d\n", points);
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
        renderer->DrawArc(Position, _scannerRange, Angle, _scannerArcWidth, ColorWhite, 10); //Scanner arc

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

        //Reinit bot. Sets port callbacks and does hardware config
        Init();

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

void Robot::Damage(f32 damage)
{
    //Some damage absorbed by the armor and shields
    f32 absorbedByShields = Shields * damage;
    f32 damageThroughShields = damage - absorbedByShields;
    Armor -= damageThroughShields * ArmorDamageMultiplier;

    //Damage absorbed by shields gets turned into heat
    Heat += absorbedByShields;
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