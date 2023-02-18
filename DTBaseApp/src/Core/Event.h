#pragma once
#include <functional>
#include "Core.h"
#include "InputKeyCodes.h"

namespace DT
{
	class Event
	{
	public:
		class Dispatcher
		{
			template<typename T>
			using EventFn = std::function<bool(T&)>;
		public:
			Dispatcher(Event& e)
				:
				m_Event(e)
			{}
		public:
			template<typename T>
			bool Dispatch(EventFn<T> f)
			{
				if (m_Event.GetType() == T::GetStaticType())
				{
					m_Event.Handled = f(*static_cast<T*>(&m_Event));
					return true;
				}
				return false;
			}
		private:
			Event& m_Event;
		};
	public:
		enum class Type
		{
			None = 0           ,
			WindowClosed       ,
			WindowResize       ,
			WindowExitSize     ,
			WindowFocus        ,
			WindowMoved        ,
			WindowRestoreDown  ,
			WindowMaximize     ,
			WindowIconify      ,
			WindowToFullscreen ,
			WindowToWindowed   ,
			KeyPressed         ,
			KeyReleased        ,
			KeyTyped           ,
			MouseButtonPressed ,
			MouseButtonReleased,
			MouseMoved         ,
			MouseLeaved        ,
			MouseScrolled
		};
		enum Category
		{
			CategoryNone        = 0,
			CategoryApplication = Bit(0),
			CategoryInput       = Bit(1),
			CategoryKeyboard    = Bit(2),
			CategoryMouse       = Bit(3),
			CategoryMouseButton = Bit(4)
		};
	public:
		virtual ~Event() = default;
	public:
		virtual Type GetType() const = 0;
		virtual const char* GetName() const = 0;
		virtual int32 GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }
		bool IsInCategory(Category category) { return GetCategoryFlags() & category; }
	public:
		bool Handled = false;
	};

	inline std::ostream& operator << (std::ostream& oss, const Event& e)
	{
		return oss << e.ToString();
	}

	/******** reduce boilerplate copy pasta ************/
	#define IMPLEMENT_CLASS_TYPE(type)                  \
		static Type GetStaticType()                     \
		{                                               \
			return Type::##type;                        \
		}                                               \
		virtual Type GetType() const override           \
		{                                               \
			return GetStaticType();                     \
		}                                               \
		virtual const char* GetName() const override    \
		{                                               \
			return #type;                               \
		}                                               \
												     
	#define IMPLEMENT_CATEGORIES(categories)            \
		virtual int32 GetCategoryFlags() const override \
		{											    \
			return int32(categories);          	        \
		}

	/** events from Mouse interaction **/
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int32 x, int32 y)
			: m_MouseX(x), m_MouseY(y)
		{}

		int32 GetX() const { return m_MouseX; }
		int32 GetY() const { return m_MouseY; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseMovedEvent Mouse Pos: [" << m_MouseX << ", " << m_MouseY << "]";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseMoved)
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryMouse)
	private:
		int32 m_MouseX;
		int32 m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(float deltaX, float deltaY)
			: m_DeltaX(deltaX), m_DeltaY(deltaY)
		{}

		float GetDeltaX() const { return m_DeltaX; }
		float GetDeltaY() const { return m_DeltaY; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseScrolledEvent WheelDelta: [" << m_DeltaX << ", " << m_DeltaY << "]";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseScrolled)
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryMouse)
	private:
		float m_DeltaX;
		float m_DeltaY;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetButtonCode() const { return m_ButtonCode; }
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryMouse)
	protected:
		MouseButtonEvent(int32 buttonCode)
			: m_ButtonCode(buttonCode)
		{}
	protected:
		int32 m_ButtonCode;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int32 buttonCode)
			: MouseButtonEvent(buttonCode)
		{}

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent buttonCode [" << m_ButtonCode << "]";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int32 buttonCode)
			: MouseButtonEvent(buttonCode)
		{}
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent buttonCode [" << m_ButtonCode << "]";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseButtonReleased)
	};

	class MouseLeavedEvent : public Event
	{
	public:
		MouseLeavedEvent() = default;
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseLeavedEvent";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseLeaved)
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryMouse)
	};

	class MouseRawInputEvent : public Event
	{
	public:
		MouseRawInputEvent(int32 accX, int32 accY)
			: m_AccX(accX), m_AccY(accY)
		{}
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseRawInputEvent (accX, accY) = (" << m_AccX << ", " << m_AccY << ")";
			return ss.str();
		}
		IMPLEMENT_CLASS_TYPE(MouseLeaved)
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryMouse)
	private:
		int32 m_AccX;
		int32 m_AccY;
	};

	/** Events from Keyboard interaction **/
	class KeyEvent : public Event
	{
	public:
		IMPLEMENT_CATEGORIES(CategoryInput | CategoryKeyboard)
		int32 GetKeyCode() const { return m_KeyCode; }
	protected:
		KeyEvent(int32 key)
			: m_KeyCode(key)
		{}
	protected:
		int32 m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(int32 key, int32 repeatCount)
			: KeyEvent(key), m_RepeatCount(repeatCount)
		{}
	public:
		IMPLEMENT_CLASS_TYPE(KeyPressed)
		int32 GetRepeatCount() const { return m_RepeatCount; }
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << typeid(KeyPressedEvent).name() << " " <<
				" keyCode["     << m_KeyCode     << "] " <<
				" repeatCount[" << m_RepeatCount << "]";
			return ss.str();
		}
	private:
		int32 m_RepeatCount;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(int32 key)
			: KeyEvent(key)
		{}
	public:
		IMPLEMENT_CLASS_TYPE(KeyReleased)
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << typeid(KeyReleasedEvent).name() << " " <<
				" keyCode[" << m_KeyCode << "]";
			return ss.str();
		}
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(int32 key)
			: KeyEvent(key)
		{}
	public:
		IMPLEMENT_CLASS_TYPE(KeyTyped)
		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << typeid(KeyTypedEvent).name() << " " <<
				" keyCode[" << m_KeyCode << "][\'" << (char)m_KeyCode << "\']";
			return ss.str();
		}
	};


	/** events from Application/Window **/
	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int32 newWidth, int32 newHeight)
			: m_NewWidth(newWidth), m_NewHeight(newHeight)
		{}

		bool IsDegenerate() const { return (m_NewWidth == 0) || (m_NewHeight == 0); }
		int32 GetWidth() const { return m_NewWidth; }
		int32 GetHeight() const { return m_NewHeight; }

		virtual std::string ToString() const override
		{
			std::stringstream ss;
			ss << "WindowResizeEvent (width, height) = (" << m_NewWidth << ", " << m_NewHeight << ")";
			return ss.str();
		}
	public:
		IMPLEMENT_CLASS_TYPE(WindowResize)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	private:
		int32 m_NewWidth;
		int32 m_NewHeight;
	};

	class WindowClosedEvent : public Event
	{
	public:
		WindowClosedEvent() = default;
	public:
		IMPLEMENT_CLASS_TYPE(WindowClosed)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	};

	class WindowRestoredDownEvent : public Event
	{
	public:
		WindowRestoredDownEvent() = default;
	public:
		IMPLEMENT_CLASS_TYPE(WindowRestoreDown)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	};

	class WindowMaximizedEvent : public Event
	{
	public:
		WindowMaximizedEvent() = default;
	public:
		IMPLEMENT_CLASS_TYPE(WindowMaximize)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	};

	class WindowToFullscreenEvent : public Event
	{
	public:
		WindowToFullscreenEvent() = default;
	public:
		IMPLEMENT_CLASS_TYPE(WindowToFullscreen)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	};

	class WindowToWindowedEvent : public Event
	{
	public:
		WindowToWindowedEvent() = default;
	public:
		IMPLEMENT_CLASS_TYPE(WindowToWindowed)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	};

	class WindowIconifiedEvent : public Event
	{
	public:
		WindowIconifiedEvent(bool iconified)
			: m_Iconified(iconified)
		{}
		bool Minimized() const { return m_Iconified; }
		bool Maximized() const { return !m_Iconified; }

		virtual std::string ToString() const override
		{
			return std::format("WindowIconified: {0}", m_Iconified);
		}
	public:
		IMPLEMENT_CLASS_TYPE(WindowIconify)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	private:
		bool m_Iconified;
	};

	class WindowFocusEvent : public Event
	{
	public:
		WindowFocusEvent(bool focused)
			: m_Focused(focused)
		{}
		bool IsFocused() const { return m_Focused; }
		bool LostFocus() const { return !m_Focused; }
	public:
		IMPLEMENT_CLASS_TYPE(WindowFocus)
		IMPLEMENT_CATEGORIES(CategoryApplication)
	private:
		bool m_Focused;
	};
}