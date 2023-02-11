#pragma once
#include <string>

namespace DT
{
	class FileDialogs
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
	};

	class MessageBoxes
	{
	public:
		static void ShowInfo(const std::string& content, const std::string& title = "Information", bool parented = false);
		static void ShowWarning(const std::string& content, const std::string& title = "Warning", bool parented = false);
		static void ShowError(const std::string& content, const std::string& title = "Error", bool parented = false);
	};
}