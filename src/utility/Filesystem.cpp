#include "Filesystem.h"
#include <filesystem>
#include <nfd.h>

std::optional<std::string> OpenFile(const char* filter)
{
	char* out = nullptr;
	nfdresult_t result = NFD_OpenDialog(filter, nullptr, &out);
	if (result == NFD_OKAY)
		return std::string(out);
	else
		return {};
}

std::optional<std::string> OpenFolder()
{
	char* out = nullptr;
	nfdresult_t result = NFD_PickFolder(nullptr, &out);
	if (result == NFD_OKAY)
		return std::string(out);
	else
		return {};
}

std::optional<std::string> SaveFile(const char* filter)
{
	char* out = nullptr;
	nfdresult_t result = NFD_SaveDialog(filter, nullptr, &out);
	if (result == NFD_OKAY)
		return std::string(out);
	else
		return {};
}