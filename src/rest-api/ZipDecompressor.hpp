#pragma once

#include <filesystem>

namespace RestApi
{

std::filesystem::path decompressZip(const std::string& zip, const std::filesystem::path& directory);

}
