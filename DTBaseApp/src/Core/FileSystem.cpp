#include "FileSystem.h"
#include <fstream>

namespace DT
{
    Buffer FileSystem::ReadFileBinary(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);
        
        if (!file.is_open())
            return Buffer();

        Buffer content((uint64)file.tellg());

        file.seekg(0);
        file.read((char*)content.Data, content.Size);

        return content;
    }

    std::string FileSystem::ReadFileText(const std::filesystem::path& filepath)
    {
        std::ifstream file(filepath, std::ios::in | std::ios::binary | std::ios::ate);

        if (!file.is_open())
            return std::string();

        std::string content;
        content.resize((uint64)file.tellg());

        file.seekg(0);
        file.read((char*)content.data(), content.size());

        return content;
    }
}