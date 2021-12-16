#include "Sound.h"
#include <filesystem>
#include <SDL_mixer.h>
#include <iostream>
#include "Config.h"
#include "BuildConfig.h"

CVar CVar_SoundVolume("Sound volume", ConfigType::Int,
    "Volume of sound effects. Range is [0, 128]",
    ConfigValue(128), //Default value
    true,  //ShowInSettings
    false, //IsFolderPath
    false, //IsFilePath
    0, 128 //Min-max
);

std::unordered_map<std::string, Mix_Music*> Sound::_instances = {};

bool Sound::Init()
{
    //Init SDL_mixer
    //Todo: Support other formats like MP3 and Ogg. Requires pulling in some other libraries.
    //i32 initFlags = 0;
    //if (Mix_Init(initFlags) == 0)
    //{
    //    std::cerr << "Failed to initialize SDL_mixer! Error: " << Mix_GetError() << "\n";
    //    return false;
    //}
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        std::cerr << "Failed to initialize SDL_mixer! Error: " << Mix_GetError() << "\n";
        return false;
    }

    return true;
}

void Sound::Shutdown()
{
    for (auto& kv : _instances)
        Mix_FreeMusic(kv.second);

    _instances.clear();
    Mix_Quit();
}

Mix_Music* Sound::GetOrLoadSound(const std::string& filename)
{
    //Check that file exists
    std::string path = BuildConfig::AssetFolderPath + "sounds/" + filename;
    if (!std::filesystem::exists(path))
    {
        std::cout << "Failed to load sound '" << std::filesystem::path(path).filename() << "'. File does not exist\n";
        return nullptr;
    }

    //Check if file is already loaded
    auto search = _instances.find(path);
    if (search != _instances.end())
        return search->second;

    //Load file and cache in memory
    Mix_Music* music = Mix_LoadMUS(path.c_str());
    if (!music)
    {
        std::cout << "Failed to load sound '" << std::filesystem::path(path).filename() << "'. Error: " << Mix_GetError() << "\n";
        return nullptr;
    }
    _instances[path] = music;
    return music;
}

bool Sound::PlaySound(const std::string& filename)
{
    if (Mix_Music* sound = GetOrLoadSound(filename))
    {
        Mix_VolumeMusic(CVar_SoundVolume.Get<i32>());
        Mix_PlayMusic(sound, 0);
        return true;
    }
    else
    {
        return false;
    }
}