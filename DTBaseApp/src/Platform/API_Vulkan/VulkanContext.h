#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"

namespace DT
{
	class VulkanContext : public RendererContext
	{
	public:
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Present() override;

		static bool IsExtensionAvailable(const char* extensionName);
		static bool IsLayerAvailable(const char* layerName);
	private:
		std::vector<const char*> BuildRequestedExtensions();
		std::vector<const char*> BuildRequestedLayers();
	private:
		inline static VkInstance s_Instance = VK_NULL_HANDLE;

		inline static std::vector<VkLayerProperties> s_AvailableLayers;
		inline static std::vector<VkExtensionProperties> s_AvailableExtensions;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
	};
}