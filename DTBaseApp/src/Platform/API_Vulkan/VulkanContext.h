#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"
#include "VulkanBuffers.h"

namespace DT
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(const Ref<Window>& window);

		virtual void Init() override;
		virtual void Shutdown() override;

		static bool IsInstanceExtensionSupported(const char* extensionName);
		static bool IsInstanceLayerSupported(const char* layerName);

		Ref<Window> GetWindow() const { return m_Window; }

		static VulkanContext& Get() { return *s_Context; }
		static VkInstance GetVulkanInstance() { return s_Context->m_Instance; }
		
		static VulkanDevice& GetCurrentDevice() { return s_Context->m_Device; }
		static VulkanPhysicalDevice& GetCurrentPhysicalDevice() { return s_Context->m_PhysicalDevice; }
		
		static VkDevice GetCurrentVulkanDevice() { return s_Context->m_Device.GetVulkanDevice(); }
		static VkPhysicalDevice GetCurrentVulkanPhysicalDevice() { return s_Context->m_PhysicalDevice.GetVulkanPhysicalDevice(); }
		
		static VkSurfaceKHR GetSurface() { return s_Context->m_Surface; }
		static VmaAllocator GetVulkanMemoryAllocator() { return s_Context->m_VulkanMemoryAllocator; }
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
		VmaAllocator m_VulkanMemoryAllocator = VK_NULL_HANDLE;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		VkSurfaceKHR m_Surface = VK_NULL_HANDLE;
		
		VulkanPhysicalDevice m_PhysicalDevice;
		VulkanDevice m_Device;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}