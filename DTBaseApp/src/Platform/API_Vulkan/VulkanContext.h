#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"
#include "VulkanDevice.h"

namespace DT
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(const Ref<Window>& window);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Present() override;

		static bool IsInstanceExtensionSupported(const char* extensionName);
		static bool IsInstanceLayerSupported(const char* layerName);

		static VkInstance GetVulkanInstance() { return s_Instance; }
	private:
		void CreateVulkanInstance();
		void SelectPhysicalDevice();
		void CreateMemoryAllocator();
	private:
		std::vector<const char*> BuildRequestedInstanceExtensions();
		std::vector<const char*> BuildRequestedInstanceLayers();
	private:
		Ref<Window> m_Window;

		inline static VkInstance s_Instance = VK_NULL_HANDLE;

		inline static std::vector<VkLayerProperties> s_AvailableInstanceLayers;
		inline static std::vector<VkExtensionProperties> s_AvailableInstanceExtensions;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		VulkanPhysicalDevice m_PhysicalDevice;
		VulkanDevice m_Device;
	};
}