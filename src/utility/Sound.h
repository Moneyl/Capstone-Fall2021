#pragma once
#include "Typedefs.h"
#include <unordered_map>
#include <string>

//Forward decl SDL_mixer types
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;

//Wrapper around SDL_mixer
class Sound
{
public:
    static bool Init();
    static void Shutdown();
    //Get sound from the sound cache. If the sound isn't in the cache, load it.
    static Mix_Music* GetOrLoadSound(const std::string& filename);
    //Play sound. ::GetOrLoadSound() handles loading it from memory/disk.
    static bool PlaySound(const std::string& filename);

private:
    static std::unordered_map<std::string, Mix_Music*> _instances;

};