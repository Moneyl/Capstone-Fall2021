#pragma once
#include "Typedefs.h"
#include "Instruction.h"
#include <unordered_map>
#include <string_view>
#include <limits>

//Ports set to their addresses in VM memory
enum class Port
{
    Spedometer = sizeof(VmValue) * 0,
    Heat = sizeof(VmValue) * 1,
    Compass = sizeof(VmValue) * 2,
    Steering = sizeof(VmValue) * 3,
    TurretShoot = sizeof(VmValue) * 4,
    TurretRotateOffset = sizeof(VmValue) * 5,
    TurretRotateAbsolute = sizeof(VmValue) * 6,
    MineLayer = sizeof(VmValue) * 7,
    MineTrigger = sizeof(VmValue) * 8,
    NumPorts = 8,
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

    //Interrupts

    //Misc
    { "INT_MAX", std::numeric_limits<VmValue>::max() },
    { "INT_MIN", std::numeric_limits<VmValue>::min() },
};