#pragma once
#include <Core/Core.h>
#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vk_mem_alloc.h>

#if DT_DEBUG
	#define VK_CALL(call)                          \
	{ 							                   \
		VkResult result = (call);                  \
		if (result != VK_SUCCESS)                  \
		{						                   \
			LOG_CRITICAL(string_VkResult(result)); \
			ASSERT(false); 		                   \
		} 						                   \
	}
#else
	#define VK_CALL(call) (call)
#endif

#ifdef DT_DEBUG
	static constexpr bool s_ValidationLayerEnabled = true;
#else
	static constexpr bool s_ValidationLayerEnabled = false;
#endif

#define VK_VALIDATION_LAYER_NAME "VK_LAYER_KHRONOS_validation"

namespace DT
{

}