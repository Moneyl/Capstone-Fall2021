#pragma once
#include "Typedefs.h"
#include "vm/VM.h"
#include "math/Vec2.h"
#include "vm/Constants.h"
#include <filesystem>
#include <memory> //For std::unique_ptr<T>
#include <array>

class Renderer;
class Arena;

//Robot tank used in the arena. Each has a single VM running a program that controls the robots hardware.
class Robot
{
public:
    Robot();

    //Per frame update
    void Update(Arena& arena, f32 deltaTime, u32 cyclesToExecute);
    //Draw robot in the window
    void Draw(Renderer* renderer);
    //Load program from asm source file
    void LoadProgramFromSource(std::string_view inFilePath);
    //Recompile program if the source file was edited since last reload
    void TryReload();
    //Path of source file
    const std::string& SourcePath() const { return _sourceFilePath; }
    //Use to uniquely identify a robot
    u64 ID() const { return _id; }
    Vec2<f32> TurretDirection() const;
    //Points of the triangle that make up the chassis
    std::array<Vec2<f32>, 3> GetChassisPoints() const;
    //Apply damage to bot. Shields and armor can mitigate this.
    void Damage(f32 damage);
    //Returns true if the point lies within the chassis
    bool PointInChassis(const Vec2<f32>& point) const;
    //Returns true if the robot chassis lies within the rectangle
    bool ChassisInRectangle(const Vec2<f32>& rectPos, const Vec2<f32>& rectSize) const;

    //Virtual machine that runs the robots logic program
    std::unique_ptr<VM> Vm = std::unique_ptr<VM>(new VM());
    Vec2<f32> Position = { 0, 0 }; //Robot position in arena
    Vec2<f32> LastPosition = { 0, 0 };
    f32 Angle = 0; //Robot chassis angle in degress
    f32 Speed = 0; //Current speed
    f32 TurretAngle = 0;
    f32 Heat = 0.0f;
    f32 Armor = ArmorBase;
    f32 MaxArmor = ArmorBase;
    f32 Engine = 1.0f;
    f32 Heatsink = 1.0f;
    f32 Shields = 0.0f;
    f32 MaxSpeed = SpeedBase;
    f32 ArmorDamageMultiplier = 1.0f;
    f32 Accuracy = 0.0f; //Accuracy of last scanner activation. Difference in degrees between the turret direction and the direction of the detected bot
    bool ShieldOn = true;
    VmValue NumMines = MaxMines;

    //Set to true when an error occurs. If true ::Update() is stopped until the error is resolved.
    bool Error = false;

    static const inline f32 TurretLength = 12.0f;
    static const inline f32 TurretShootFrequency = 1.0f / 5.0f; //Max shots / second of turrets
    static const inline VmValue TurretShootAngleControl = 4; //Num of degrees in either direction bullets can be shifted towards when shooting
    static const inline f32 MineLayerFrequency = 1.0f / 2.0f; //Max mines that can be laid per second
    static const inline f32 ChassisSize = 10.0f;
    static const inline VmValue MaxMines = 10; //Todo: Make this configurable with #config directives
    static const inline f32 RadarSonarRange = 150.0f;
    static const inline f32 RadarSonarFrequency = 1.0f / 3.0f;
    static const inline f32 ScannerFrequency = 1.0f / 3.0f;
    static const inline f32 MaxHeat = 500.0f;
    static const inline f32 HeatDamageThreshold = MaxHeat * 0.8f;
    static const inline f32 OverHeatDamageFrequency = 0.25f;
    static const inline f32 HeatPerTurretShot = 2.5f;
    static const inline u32 MaxConfigPoints = 12; //Total config points to be spread between scanner, turret, armor, engine, heatsinks, mines, and shield
    static const inline f32 MineDamage = 0.15f; //Damage caused by mines at the center of the detonation. Damage decreases with distance.
    static const inline f32 ArmorBase = 12.0f; //Base armor level that the armor config directive modifies
    static const inline f32 SpeedBase = 150.0f; //Base max speed that the engine config directive modifies
private:
    //Called by the VM when ports are read (ipo) and written (opo)
    //This updates the ports with the latest hardware state on reads, and lets hardware respond to writes.
    void OnPortRead(Port port, f32 deltaTime);
    void OnPortWrite(Port port, VmValue value, f32 deltaTime);
    void Init();

    //Setup hardware based on values from #config directions in assembly
    void SetupScanner(VmValue points);
    void SetupTurret(VmValue points);
    void SetupArmor(VmValue points);
    void SetupEngine(VmValue points);
    void SetupHeatsink(VmValue points);
    void SetupMines(VmValue points);
    void SetupShield(VmValue points);

    std::filesystem::file_time_type _sourceFileLastWriteTime;
    std::string _sourceFilePath;
    u64 _id;
    f32 _turretShootTimer = 0.0f; //Limits turret fire rate
    f32 _mineLayerTimer = 0.0f; //Limits mine layer rate
    f32 _radarTimer = 0.0f;
    f32 _sonarTimer = 0.0f;
    f32 _scannerTimer = 0.0f;
    f32 _scannerArcWidth = 32.0f; //[0, 64]
    f32 _scannerRange = 250.0f;
    f32 _turretDamage = 1.0f;

    //True if the hardware was used this frame. Used by renderer to draw circles/arcs/etc.
    bool _sonarOn = false;
    bool _radarOn = false;
    bool _scannerOn = false;
    Arena* _arena = nullptr;
};