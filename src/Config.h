#pragma once
#include "Typedefs.h"
#include "utility/String.h"
#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Vec4.h"
#include <string_view>
#include <algorithm>
#include <optional>
#include <variant>
#include <vector>

enum class ConfigType : u32;
class CVar;

//Loads and saves CVars
class Config
{
public:
    //Load config variables from file. Discards values in memory
    void Load();
    //Saves config variables to config files.
    void Save();
    //Returns a variable if it exists. Throws an exception if it doesn't exist. Case insensitive.
    CVar& GetCvar(std::string_view variableName) const;
    //Get instance of this class. Use this instead of manually creating an instance and passing it around.
    static Config* Get();
};

//CVar data type
enum class ConfigType : u32
{
    Int,
    Uint,
    Float,
    Bool,
    String,
    List, //Note: This currently only supports a list of strings
    Invalid,
};

//Value storage for config variables
using ConfigValue = std::variant<i32, u32, f32, bool, std::string, Vec2<f32>, Vec3<f32>, Vec4<f32>, std::vector<std::string>>;

//Config variable. Defined at compile time and automatically saved/loaded by Config into Config.xml
class CVar
{
public:
    //All CVar instances. Defined at compile time. Filled at runtime by static initialization.
    static std::vector<CVar*> Instances;

    const std::string Name; //Case insensitive
    const ConfigType Type;
    const std::string Description;
    const ConfigValue Default;
    ConfigValue Value;
    const bool ShowInSettings;
    const bool IsFolderPath;
    const bool IsFilePath;
    const std::optional<f32> Min;
    const std::optional<f32> Max;

    CVar(std::string_view name, ConfigType type, std::string_view description, ConfigValue defaultValue, bool showInSettings = false, bool isFolderPath = false, bool isFilePath = false,
        std::optional<f32> min = {}, std::optional<f32> max = {}) :
        Name(name), Type(type), Description(description), Default(defaultValue), Value(defaultValue), ShowInSettings(showInSettings), IsFolderPath(isFolderPath),
        IsFilePath(isFilePath), Min(min), Max(max)
    {
        //Add to list if it's not a repeat definition
        auto search = std::find_if(Instances.begin(), Instances.end(), [&](const CVar* cvar) { return String::EqualIgnoreCase(cvar->Name, Name); });
        if (search == Instances.end())
            Instances.push_back(this);
    }

    template<typename T>
    T& Get()
    {
        return std::get<T>(Value);
    }
};



/*Config variable definitions*/
extern CVar CVar_DataPath;
extern CVar CVar_RecentProjects;