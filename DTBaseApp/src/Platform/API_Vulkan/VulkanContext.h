#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"

namespace DT
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(const Ref<Window>& window);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Present() override;

		bool IsInstanceExtensionSupported(const char* extensionName);
		bool IsInstanceLayerSupported(const char* layerName);
		
		const std::vector<VkPhysicalDevice>& GetAvailablePhysicalDevices() const { return m_AvailablePhysicalDevices; }

		static VkInstance GetVulkanInstance() { return s_Context->m_Instance; }

		static VulkanContext& Get() { return *s_Context; }
	private:
		void CreateVulkanInstance();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateMemoryAllocator();
		void CreateLogicalDevice();
	private:
		std::vector<const char*> BuildRequestedInstanceExtensions();
		std::vector<const char*> BuildRequestedInstanceLayers();
	private:
		inline static VulkanContext* s_Context = nullptr;

		Ref<Window> m_Window;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VulkanPhysicalDevice m_PhysicalDevice;
		VulkanSwapchain m_Swapchain;
		VulkanDevice m_Device;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}