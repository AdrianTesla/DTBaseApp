#include "Log.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace DT
{
	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %v%$");

		s_DefaultLogger = spdlog::stdout_color_mt("VulkanLearning");
		s_DefaultLogger->set_level(spdlog::level::trace);
	}
}