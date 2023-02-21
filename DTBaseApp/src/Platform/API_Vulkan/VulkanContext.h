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
		virtual ~VulkanContext();

		virtual void Init() override;
		virtual void Present() override;
		virtual void OnWindowResize() override;

		virtual void DrawFrameTest() override;
		
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
		static VulkanSwapchain& GetSwapchain() { return s_Context->m_Swapchain; }
		
		static VmaAllocator GetVulkanMemoryAllocator() { return s_Context->m_VulkanMemoryAllocator; }
		static uint32 GetCurrentFrame() { return s_Context->m_CurrentFrame; }
	private:
		void CreateVulkanInstance();
		void CreateWindowSurface();
		void SelectPhysicalDevice();
		void CreateMemoryAllocator();
		void CreateLogicalDevice();
		void CreateSwapchain();
		void CreateSyncObjects();

		void CreateGraphicsPipeline();
		void CreateBuffers();
		void CreateCommandBuffers();
		void CreateDescriptorPools();
		void CreateDescriptorSets();
		void UpdateUniformBuffers();
		void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);
		void ExecuteCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue);
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
		VulkanSwapchain m_Swapchain;
		
		InFlight<VkCommandBuffer> m_GraphicsCommandBuffers;
		InFlight<VkFence> m_PreviousFrameFinishedFences;
		InFlight<VkSemaphore> m_RenderCompleteSemaphores;

		Ref<VulkanShader> m_Shader;
		Ref<VulkanPipeline> m_PipelineFill;
		Ref<VulkanPipeline> m_PipelineWireframe;

		Ref<VulkanVertexBuffer> m_VertexBuffer;
		Ref<VulkanIndexBuffer> m_IndexBuffer;

		InFlight<Ref<VulkanUniformBuffer>> m_UniformBuffers;

		VkDescriptorPool m_DescriptorPool = VK_NULL_HANDLE;
		InFlight<VkDescriptorSet> m_DescriptorSets;

		uint32 m_CurrentFrame = 0u;

		std::vector<VkPhysicalDevice> m_AvailablePhysicalDevices;
		std::vector<VkLayerProperties> m_AvailableInstanceLayers;
		std::vector<VkExtensionProperties> m_AvailableInstanceExtensions;
	};
}