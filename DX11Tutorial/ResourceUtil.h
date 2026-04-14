#pragma once
#include <string>
#include <filesystem>

bool BuildBlockKeyFromPath(const std::filesystem::path& filePath, std::string& outBlockKey);
bool NormalizeTextureKey(const char* inKey, std::string& outKey);