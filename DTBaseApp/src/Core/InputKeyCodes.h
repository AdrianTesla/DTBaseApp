#pragma once
#include "Core.h"

namespace DT
{
	using KeyCode = uint16;

	/** Keyboard Codes (from GLFW) **/
	namespace Key
	{
		enum : KeyCode
		{
			Space         = 32,
			Apostrophe    = 39, //  '
			Comma         = 44, //  ,
			Minus         = 45, //  -
			Period        = 46, //  .
			Slash         = 47, //  /
			_0            = 48,
			_1            = 49,
			_2            = 50,
			_3            = 51,
			_4            = 52,
			_5            = 53,
			_6            = 54,
			_7            = 55,
			_8            = 56,
			_9            = 57,
			Semicolon     = 59,  // ;
			Equal         = 61,  // =
			A             = 65,
			B             = 66,
			C             = 67,
			D             = 68,
			E             = 69,
			F             = 70,
			G             = 71,
			H             = 72,
			I             = 73,
			J             = 74,
			K             = 75,
			L             = 76,
			M             = 77,
			N             = 78,
			O             = 79,
			P             = 80,
			Q             = 81,
			R             = 82,
			S             = 83,
			T             = 84,
			U             = 85,
			V             = 86,
			W             = 87,
			X             = 88,
			Y             = 89,
			Z             = 90,
			LeftBracket   = 91,  /* [ */
			Backslash     = 92,  /* \ */
			RightBracket  = 93,  /* ] */
			GraveAccent   = 96,  /* ` */
			World1        = 161, /* non-US #1 */
			World2        = 162, /* non-US #2 */
			Escape        = 256,
			Enter         = 257,
			Tab           = 258,
			Backspace     = 259,
			Insert        = 260,
			Delete        = 261,
			Right         = 262,
			Left          = 263,
			Down          = 264,
			Up            = 265,
			PageUp        = 266,
			PageDown      = 267,
			Home          = 268,
			End           = 269,
			CapsLock      = 280,
			ScrollLock    = 281,
			NumLock       = 282,
			PrintScreen   = 283,
			Pause         = 284,
			F1            = 290,
			F2            = 291,
			F3            = 292,
			F4            = 293,
			F5            = 294,
			F6            = 295,
			F7            = 296,
			F8            = 297,
			F9            = 298,
			F10           = 299,
			F11           = 300,
			F12           = 301,
			F13           = 302,
			F14           = 303,
			F15           = 304,
			F16           = 305,
			F17           = 306,
			F18           = 307,
			F19           = 308,
			F20           = 309,
			F21           = 310,
			F22           = 311,
			F23           = 312,
			F24           = 313,
			F25           = 314,
			Pad0          = 320,
			Pad1          = 321,
			Pad2          = 322,
			Pad3          = 323,
			Pad4          = 324,
			Pad5          = 325,
			Pad6          = 326,
			Pad7          = 327,
			Pad8          = 328,
			Pad9          = 329,
			PadDecimal    = 330,
			PadDivide     = 331,
			PadMultiply   = 332,
			PadSubtract   = 333,
			PadAdd        = 334,
			PadEnter      = 335,
			PadEqual      = 336,
			LeftShift     = 340,
			LeftCTRL      = 341,
			LeftAlt       = 342,
			LeftSuper     = 343, // like windows home
			RightShift    = 344,
			RightCTRL     = 345,
			RightAlt      = 346,
			RightSuper    = 347,
			Menu          = 348
		};
	}

	using MouseCode = uint16;

	/**** Mouse Codes (from GLFW) ***/
	namespace Mouse
	{
		enum : MouseCode
		{
			Button1 = 0,
			Button2 = 1,
			Button3 = 2,
			Button4 = 3,
			Button5 = 4,
			Button6 = 5,
			Button7 = 6,
			Button8 = 7,
			Left    = Button1,
			Right   = Button2,
			Middle  = Button3
		};
	}
}