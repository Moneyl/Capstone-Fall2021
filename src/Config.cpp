#include "Config.h"
#include "utility/String.h"
#include <pugixml.hpp>
#include <filesystem>
#include <stdexcept>
#include <fstream>

const std::string mainConfigPath = "./Config.xml";
std::vector<CVar*> CVar::Instances = {};
const u32 ConfigFileFormatVersion = 1;

void Config::Load()
{
    //If config file doesn't exist generate a default one
    if (!std::filesystem::exists(mainConfigPath))
    {
        Save();
        return;
    }

    //Load config
    pugi::xml_document doc;
    pugi::xml_parse_result result = doc.load_file(mainConfigPath.c_str());
    if (!result)
        return;

    //Get config format version
    pugi::xml_node config = doc.root().child("Config");
    const pugi::xml_attribute formatVersion = config.attribute("FormatVersion");
    if (!formatVersion || formatVersion.as_int() != ConfigFileFormatVersion)
    {
        //Unsupported version. Delete and resave config file using default cvar values to fix it.
        std::filesystem::remove(mainConfigPath);
        Save();
        return;
    }

    //Loop through config variables and parse them
    u32 i = 0;
    for (pugi::xml_node cvarXml : config.children("CVar"))
    {
        const pugi::xml_attribute name = cvarXml.attribute("Name");
        if (name.empty())
        {
            printf("Skipped variable %d in '%s'. Must have an attribute 'Name' and a value inside its braces.\n", i, mainConfigPath.c_str());
            i++;
            continue;
        }

        //Find cvar instance
        auto find = std::find_if(CVar::Instances.begin(), CVar::Instances.end(), [&](const CVar* cvar) { return String::EqualIgnoreCase(cvar->Name, name.value()); });
        if (find == CVar::Instances.end())
        {
            printf("Skipped variable %d in '%s'. No variable definition found. Variables must be defined at compile time.\n", i, mainConfigPath.c_str());
            i++;
            continue;
        }
        CVar& var = **find;

        //Ensure xml has a value
        if (var.Type != ConfigType::List && !cvarXml.value())
        {
            printf("Skipped variable %d in '%s'. Value missing.\n", i, mainConfigPath.c_str());
            i++;
            continue;
        }

        //Parse variable value
        switch (var.Type)
        {
        case ConfigType::Int:
            var.Value = cvarXml.text().as_int();
            break;
        case ConfigType::Uint:
            var.Value = cvarXml.text().as_uint();
            break;
        case ConfigType::Float:
            var.Value = cvarXml.text().as_float();
            break;
        case ConfigType::Bool:
            var.Value = cvarXml.text().as_bool();
            break;
        case ConfigType::String:
            var.Value = std::string(cvarXml.text().as_string());
            break;
        case ConfigType::List:
        {
            //Read list of values to vector. No values is treated as an empty list
            std::vector<std::string> values = {};
            for (pugi::xml_node subvalue : cvarXml.children("Item"))
            {
                values.push_back(subvalue.text().as_string());
            }

            var.Value = values;
        }
        break;
        case ConfigType::Invalid:
        default:
            throw std::runtime_error("Unsupported type '" + std::to_string((int)var.Type) + "' detected. Skipping.");
            break;
        }

        i++;
    }

    //Save so any missing values are set to their defaults
    Save();
}

void Config::Save()
{
    pugi::xml_document doc;
    pugi::xml_node config = doc.append_child("Config");
    pugi::xml_attribute format = config.append_attribute("FormatVersion");
    format.set_value(ConfigFileFormatVersion);

    for (CVar* var : CVar::Instances)
    {
        if (var->Type >= ConfigType::Invalid)
            continue;

        pugi::xml_node cvarXml = config.append_child("CVar");
        pugi::xml_attribute name = cvarXml.append_attribute("Name");
        name.set_value(var->Name.c_str());

        //Set value
        switch (var->Type)
        {
        case ConfigType::Int:
            cvarXml.text().set(var->Get<i32>());
            break;
        case ConfigType::Uint:
            cvarXml.text().set(var->Get<u32>());
            break;
        case ConfigType::Float:
            cvarXml.text().set(var->Get<f32>());
            break;
        case ConfigType::Bool:
            cvarXml.text().set(var->Get<bool>());
            break;
        case ConfigType::String:
            cvarXml.text().set(var->Get<std::string>().c_str());
            break;
        case ConfigType::List:
        {
            for (std::string& str : var->Get<std::vector<std::string>>())
            {
                pugi::xml_node item = cvarXml.append_child("Item");
                item.text().set(str.c_str());
            }
        }
        break;
        case ConfigType::Invalid:
        default:
            //Shouldn't be possible due to type check at top of loop
            throw std::runtime_error("Unsupported type '" + std::to_string((int)var->Type) + "' detected. This shouldn't be able to happen...");
            break;
        }
    }

    //Save to file
    doc.save_file(mainConfigPath.c_str());
}

CVar& Config::GetCvar(std::string_view variableName) const
{
    for (CVar* var : CVar::Instances)
        if (String::EqualIgnoreCase(var->Name, variableName))
            return *var;

    throw std::runtime_error("Requested non-existent config var '" + std::string(variableName) + "'");
}

Config* Config::Get()
{
    static Config gConfig;
    return &gConfig;
}