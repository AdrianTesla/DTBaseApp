#pragma once
#include "spdlog/spdlog.h"

namespace DT
{
	class Log
	{
	public:
		static void Init();
		static void Shutdown();

		static std::shared_ptr<spdlog::logger>& GetDefaultLogger() { return s_DefaultLogger; }
	private:
		inline static std::shared_ptr<spdlog::logger> s_DefaultLogger;
	};

#if DT_ENABLE_LOGGING
	#define LOG_TRACE(...)    ::DT::Log::GetDefaultLogger()->trace(__VA_ARGS__)
	#define LOG_INFO(...)     ::DT::Log::GetDefaultLogger()->info(__VA_ARGS__)
	#define LOG_WARN(...)     ::DT::Log::GetDefaultLogger()->warn(__VA_ARGS__)
	#define LOG_ERROR(...)    ::DT::Log::GetDefaultLogger()->error(__VA_ARGS__)
	#define LOG_CRITICAL(...) ::DT::Log::GetDefaultLogger()->critical(__VA_ARGS__)
#else
	#define LOG_TRACE(...)
	#define LOG_INFO(...)
	#define LOG_WARN(...)
	#define LOG_ERROR(...)
	#define LOG_CRITICAL(...)
#endif
}