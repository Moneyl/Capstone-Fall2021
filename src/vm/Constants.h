#pragma once
#include "Typedefs.h"
#include "Instruction.h"
#include <unordered_map>
#include <string_view>
#include <limits>

//Ports used to communicate with hardware attached to the robot
//Ports are stored at the start of VM memory. Their address is simply their value * sizeof(VmValue)
//Read write them with ipo and opo
enum class Port
{
    Spedometer,
    Heat,
    Compass,
    Steering,
    TurretShoot,
    TurretRotateOffset,
    TurretRotateAbsolute,
    MineLayer,
    MineTrigger,
    Sonar,
    Radar,
    Scanner,
    ScannerArc,
    NumPorts,
};

//Built in assembly constants
static std::unordered_map<std::string_view, VmValue> BuiltInConstants =
{
    //Port addresses
    { "P_SPEDOMETER", (VmValue)Port::Spedometer },
    { "P_HEAT", (VmValue)Port::Heat },
    { "P_COMPASS", (VmValue)Port::Compass },
    { "P_STEERING", (VmValue)Port::Steering },
    { "P_SHOOT", (VmValue)Port::TurretShoot },
    { "P_TURRET_OFS", (VmValue)Port::TurretRotateOffset },
    { "P_TURRET_ABS", (VmValue)Port::TurretRotateAbsolute },
    { "P_MINELAYER", (VmValue)Port::MineLayer },
    { "P_MINETRIGGER", (VmValue)Port::MineTrigger },
    { "P_SONAR", (VmValue)Port::Sonar },
    { "P_RADAR", (VmValue)Port::Radar },
    { "P_SCANNER", (VmValue)Port::Scanner },
    { "P_SCAN_ARC", (VmValue)Port::ScannerArc },

    //Interrupts

    //Misc
    { "INT_MAX", std::numeric_limits<VmValue>::max() },
    { "INT_MIN", std::numeric_limits<VmValue>::min() },
};