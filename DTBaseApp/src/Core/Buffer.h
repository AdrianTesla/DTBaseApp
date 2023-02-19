#pragma once
#include "Core.h"

namespace DT
{
	// non owning raw buffer, release manually
	struct Buffer
	{
		void* Data = nullptr;
		uint64 Size = 0u;

		Buffer() = default;

		~Buffer() = default;

		Buffer(const Buffer& other) = default;

		Buffer(uint64 size)
		{
			Allocate(size);
		}

		Buffer(uint64 size, void* data)
			: Size(size), Data(data)
		{}

		static Buffer Copy(Buffer other)
		{
			Buffer result(other.Size);
			memcpy(result.Data, other.Data, other.Size);
			return result;
		}

		void Allocate(uint64 size)
		{
			Release();

			if (size == 0u)
				return;

			Data = new uint8[size];
			Size = size;
		}

		void Clear()
		{
			if (Data)
				memset(Data, 0, Size);
		}

		void Release()
		{
			delete[] Data;
			Data = nullptr;
			Size = 0u;
		}

		template<typename T>
		T* As()
		{
			return (T*)Data;
		}

		template<typename T>
		T& Read(uint32 byteOffset = 0u)
		{
			return *(T*)((uint8*)Data + byteOffset);
		}

		template<typename T>
		const T& Read(uint32 byteOffset = 0u) const
		{
			return *(T*)((uint8*)Data + byteOffset);
		}

		void Write(const void* data, uint64 size, uint64 byteOffset)
		{
			memcpy((uint8*)Data + byteOffset, data, size);
		}

		operator bool() const
		{
			return (Data != nullptr) && (Size != 0u);
		}

		uint8& operator[](uint32 index)
		{
			return ((uint8*)Data)[index];
		}

		uint8 operator[](uint32 index) const
		{
			return ((uint8*)Data)[index];
		}
	};
}