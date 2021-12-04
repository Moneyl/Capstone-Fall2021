#pragma once
#include "Typedefs.h"
#include <optional>
#include <string>

//Wrappers around native file dialog
std::optional<std::string> OpenFile(const char* filter = nullptr);
std::optional<std::string> OpenFolder();
std::optional<std::string> SaveFile(const char* filter = nullptr);