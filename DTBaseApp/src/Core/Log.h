#pragma once
#include "spdlog/spdlog.h"

namespace DT
{
	class Log
	{
	public:
		static void Init();
		static std::shared_ptr<spdlog::logger>& GetDefaultLogger() { return s_DefaultLogger; }
	private:
		inline static std::shared_ptr<spdlog::logger> s_DefaultLogger;
	};

	#define LOG_TRACE(...)    ::DT::Log::GetDefaultLogger()->trace(__VA_ARGS__)
	#define LOG_INFO(...)     ::DT::Log::GetDefaultLogger()->info(__VA_ARGS__)
	#define LOG_WARN(...)     ::DT::Log::GetDefaultLogger()->warn(__VA_ARGS__)
	#define LOG_ERR(...)      ::DT::Log::GetDefaultLogger()->error(__VA_ARGS__)
	#define LOG_CRITICAL(...) ::DT::Log::GetDefaultLogger()->critical(__VA_ARGS__)
}