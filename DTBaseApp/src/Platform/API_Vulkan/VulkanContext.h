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
		virtual void DrawFrameTest() override;
		
		bool IsInstanceExtensionSupported(const char* extensionName) const;
		bool IsInstanceLayerSupported(const char* layerName) const;
		
		Ref<Window> GetWindow() const { return m_Window; }
		
		VkSurfaceKHR GetSurface() const { return m_Surface; }

		static VulkanContext& Get() { return *s_Context; }
		static VkInstance GetVulkanInstance() { return s_Context->m_Instance; }
		
		static VulkanDevice& GetCurrentDevice() { return s_Context->m_Device; }
		static VulkanPhysicalDevice& GetCurrentPhysicalDevice() { return s_Context->m_PhysicalDevice; }

		static VkDevice GetCurrentVulkanDevice() { return s_Context->m_Device.GetVulkanDevice(); }
		static VkPhysicalDevice GetCurrentVulkanPhysicalDevice() { return s_Context->m_PhysicalDevice.GetVulkanPhysicalDevice(); }
	private:
		void CreateVulkanInstance();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateMemoryAllocator();
		void CreateLogicalDevice();
		void CreateSwapchain();

		void CreateGraphicsPipeline();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateCommandBuffer();
		void RecordCommandBuffers(VkCommandBuffer commandBuffer, uint32 imageIndex);
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

		VkPipeline m_Pipeline = VK_NULL_HANDLE;
		VkPipelineLayout m_PipelineLayout = VK_NULL_HANDLE;
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> m_Framebuffers;
		VkCommandBuffer m_GraphicsCommandBuffer = VK_NULL_HANDLE;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}