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

		bool IsInstanceExtensionSupported(const char* extensionName) const;
		bool IsInstanceLayerSupported(const char* layerName) const;
		
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VkPhysicalDevice GetCurrentPhysicalDevice() { return m_PhysicalDevice.GetPhysicalDevice(); }
		Ref<Window> GetWindow() const { return m_Window; }

		static VulkanContext& Get() { return *s_Context; }
		static VkInstance GetVulkanInstance() { return s_Context->m_Instance; }
	private:
		void CreateVulkanInstance();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateMemoryAllocator();
		void CreateLogicalDevice();
		void CreateSwapchain();
	private:
		std::vector<const char*> BuildRequestedInstanceExtensions();
		std::vector<const char*> BuildRequestedInstanceLayers();
	private:
		inline static VulkanContext* s_Context = nullptr;

		Ref<Window> m_Window;

		VkInstance m_Instance = VK_NULL_HANDLE;
		VmaAllocator m_MemoryAllocator = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		
		VulkanPhysicalDevice m_PhysicalDevice;
		VulkanDevice m_Device;
		VulkanSwapchain m_Swapchain;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}