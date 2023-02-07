#include "Input.h"
#include "Application.h"
#include "Platform/WindowsWindow.h"

namespace DT
{
    bool Input::KeyIsPressed(KeyCode key)
    {
        WindowsWindow* window = (WindowsWindow*)&Application::Get().GetWindow();
        return window->KeyIsPressed(key);
    }

    bool Input::MouseIsPressed(MouseCode button)
    {
        WindowsWindow* window = (WindowsWindow*)&Application::Get().GetWindow();
        return window->MouseIsPressed(button);
    }

    int32 Input::GetMouseX()
    {
        return Application::Get().GetWindow().GetMouseX();
    }

    int32 Input::GetMouseY()
    {
        return Application::Get().GetWindow().GetMouseY();
    }
}