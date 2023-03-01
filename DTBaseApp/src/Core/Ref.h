#pragma once
#include "Core/Core.h"

namespace DT
{
	class RefCounted
	{
	private:
		uint32 m_RefCount = 0u;

		template<typename T2>
		friend class Ref;
	};

	template<typename T>
	class Ref
	{
		static_assert(std::is_base_of<RefCounted, T>(), "Creating a Ref<> on a type that is not RefCounted!");
	public:
		Ref() 
			: m_Instance(nullptr) 
		{}

		Ref(std::nullptr_t null) 
			: m_Instance(nullptr)
		{}

		Ref(T* instance)
			: m_Instance(instance)
		{
			IncrementRefCount();
		}

		Ref(const Ref& other)
			: m_Instance(other.m_Instance)
		{
			IncrementRefCount();
		}

		Ref(Ref&& other) noexcept
		{
			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
		}

		template<typename T2>
		Ref(const Ref<T2>& other)
		{
			m_Instance = (T*)other.m_Instance;
			IncrementRefCount();
		}

		template<typename T2>
		Ref(Ref<T2>&& other)
		{
			m_Instance = (T*)other.m_Instance;
			other.m_Instance = nullptr;
		}

		~Ref()
		{
			DecrementRefCount();
		}

		Ref& operator=(const Ref& other)
		{
			other.IncrementRefCount();
			DecrementRefCount();
			m_Instance = other.m_Instance;
			return *this;
		}

		Ref& operator=(Ref&& other) noexcept
		{
			DecrementRefCount(); /// TODO: UHMMMM NOT SURE.
			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}

		template<typename T2>
		Ref& operator=(const Ref<T2>& other)
		{
			other.IncrementRefCount();
			DecrementRefCount();
			m_Instance = (T*)other.m_Instance;
			return *this;
		}

		template<typename T2>
		Ref& operator=(Ref<T2>&& other) noexcept
		{
			DecrementRefCount(); /// TODO: UHMMMM NOT SURE.
			m_Instance = other.m_Instance;
			other.m_Instance = nullptr;
			return *this;
		}
		
		void Reset(T* newInstance = nullptr)
		{
			DecrementRefCount();
			m_Instance = newInstance;
		}

		const T* operator->() const { return m_Instance; }
		T* operator->() { return m_Instance; }

		const T& operator*() const { return *m_Instance; }
		T& operator*() { return *m_Instance; }

		bool operator==(const Ref& other) const { return m_Instance == other.m_Instance; }
		bool operator!=(const Ref& other) const { return m_Instance != other.m_Instance; }

		template<typename T2>
		bool operator==(const Ref<T2>& other) const { return m_Instance == (T*)other.m_Instance; }

		template<typename T2>
		bool operator!=(const Ref<T2>& other) const { return m_Instance != (T*)other.m_Instance; }

		const T* Get() const { return m_Instance; }
		T* Get() { return m_Instance; }

		operator bool() const { return m_Instance != nullptr; }
		bool Empty() const { return m_Instance == nullptr; }

		uint32 GetRefCount() const { return m_Instance ? m_Instance->m_RefCount : 0u; }

		template<typename T2>
		Ref<T2> As() const
		{
			return Ref<T2>(*this);
		}

		template<typename ...Args>
		static Ref Create(Args&&...args)
		{
			return Ref(new T(std::forward<Args>(args)...));
		}
	private:
		void IncrementRefCount() const
		{
			if (m_Instance)
				m_Instance->m_RefCount++;
		}
		void DecrementRefCount() const
		{
			if (m_Instance)
			{
				m_Instance->m_RefCount--;
				if (m_Instance->m_RefCount == 0u)
				{
					delete m_Instance;
					m_Instance = nullptr;
				}
			}
		}
	private:
		mutable T* m_Instance;

		template<typename T2>
		friend class Ref;

	};

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ...Args>
	constexpr std::unique_ptr<T> MakeScope(Args...args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}
}