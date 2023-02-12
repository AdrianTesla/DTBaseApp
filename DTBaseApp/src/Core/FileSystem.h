#pragma once
#include "Buffer.h"
#include <filesystem>

namespace DT
{
	class FileSystem
	{
	public:
		static Buffer ReadFileBinary(const std::filesystem::path& filepath);
		static std::string ReadFileText(const std::filesystem::path& filepath);
	};
}