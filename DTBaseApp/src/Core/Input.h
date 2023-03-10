#pragma once
#include "InputKeyCodes.h"

namespace DT
{
	/* platform independent user input system */
	class Input
	{
	public:
		static bool KeyIsPressed(KeyCode key);
		static bool MouseIsPressed(MouseCode button);
		static int32 GetMouseX();
		static int32 GetMouseY();
	};
}