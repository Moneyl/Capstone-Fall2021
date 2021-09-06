#pragma once
#include "src/Typedefs.h"

//Windows specific
#include <winsdkver.h>
#define _WIN32_WINNT 0x0601 //Target windows 7
#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <synchapi.h> //For windows Sleep()

//Windows specific

//Todo: Add support for linux and mac os. One way this might be done is to define functions here, then have separate .cpp files with the implementations for each platform
void ThreadSleep(u32 ms)
{
    Sleep(ms);
}