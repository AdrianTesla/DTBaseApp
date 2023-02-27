#include "Platform/PlatformUtils.h"

#include "Core/Application.h"
#include <Windows.h>
#include <commdlg.h>

#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

namespace DT
{
	/********************** File Dialogs *************************/
	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAME ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		HWND hWnd = (HWND)Application::Get().GetWindow().GetNativeWindow();

		ofn.hwndOwner    = hWnd;
		ofn.lStructSize  = sizeof(OPENFILENAME);
		ofn.lpstrFile    = szFile;
		ofn.nMaxFile     = sizeof(szFile);
		ofn.lpstrFilter  = filter;
		ofn.nFilterIndex = 1u;
		ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileName(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAME ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));

		HWND hWnd = (HWND)Application::Get().GetWindow().GetNativeWindow();

		ofn.hwndOwner    = hWnd;
		ofn.lStructSize  = sizeof(OPENFILENAME);
		ofn.lpstrFile    = szFile;
		ofn.nMaxFile     = sizeof(szFile);
		ofn.lpstrFilter  = filter;
		ofn.nFilterIndex = 1u;
		ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		
		if (GetSaveFileName(&ofn) == TRUE)
			return ofn.lpstrFile;

		return std::string();
	}

	/********************** Message Boxes ************************/
	void MessageBoxes::ShowInfo(const std::string& content, const std::string& title, bool parented)
	{
		MessageBox(parented ? (HWND)Application::Get().GetWindow().GetNativeWindow() : nullptr, content.c_str(), title.c_str(), MB_ICONINFORMATION);
	}

	void MessageBoxes::ShowWarning(const std::string& content, const std::string& title, bool parented)
	{
		MessageBox(parented ? (HWND)Application::Get().GetWindow().GetNativeWindow() : nullptr, content.c_str(), title.c_str(), MB_ICONWARNING);
	}

	void MessageBoxes::ShowError(const std::string& content, const std::string& title, bool parented)
	{
		MessageBox(parented ? (HWND)Application::Get().GetWindow().GetNativeWindow() : nullptr, content.c_str(), title.c_str(), MB_ICONERROR);
	}
}