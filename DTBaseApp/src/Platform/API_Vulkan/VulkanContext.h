#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"
#include "VulkanDevice.h"
#include "VulkanSwapchain.h"
#include "VulkanPipeline.h"

namespace DT
{
	class VulkanContext : public RendererContext
	{
	public:
		VulkanContext(const Ref<Window>& window);
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Present() override;
		virtual void OnWindowResize() override;

		virtual void DrawFrameTest() override;
		
		bool IsInstanceExtensionSupported(const char* extensionName) const;
		bool IsInstanceLayerSupported(const char* layerName) const;
		
		static uint32 GetCurrentFrame() { return s_Context->m_CurrentFrame; }

		Ref<Window> GetWindow() const { return m_Window; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }
		VulkanSwapchain& GetSwapchain() { return m_Swapchain; }

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
		void CreateSyncObjects();

		void CreateGraphicsPipeline();
		void CreateCommandBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);
		void ExecuteCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
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
		
		InFlight<VkCommandBuffer> m_GraphicsCommandBuffers;
		InFlight<VkFence> m_PreviousFrameFinishedFences;
		InFlight<VkSemaphore> m_RenderCompleteSemaphores;

		Ref<VulkanPipeline> m_PipelineFill;
		Ref<VulkanPipeline> m_PipelineWireframe;

		uint32 m_CurrentFrame = 0u;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}