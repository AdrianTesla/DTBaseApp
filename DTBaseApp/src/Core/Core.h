#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <format>
#include <iostream>

using uint8  = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8  = int8_t;
using int16 = int16_t;
using int32 = int32_t;
using int64 = int64_t;

#include "Log.h"
#include "Timer.h"

#ifdef DT_DEBUG
	#define ASSERT(condition) if(!condition) { __debugbreak(); }
#else
	#define ASSERT(condition)
#endif

#define BIND_FUNC(x) [this](auto&&... args) -> decltype(auto) { return this->x(std::forward<decltype(args)>(args)...); }


template<typename T>
static constexpr auto Bit(T x)
{
	return (T(1) << x);
}

namespace DT
{
	void InitializeCore();
	void ShutdownCore();
}