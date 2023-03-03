#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"

namespace DT
{
	enum class QueueType
	{
		Graphics,
		Transfer,
		Compute,
		Present
	};

	struct QueueFamilyIndices
	{
		std::optional<uint32> GraphicsIndex;
		std::optional<uint32> TransferIndex;
		std::optional<uint32> ComputeIndex;
		std::optional<uint32> PresentIndex;
	};

	struct PhysicalDeviceSupportDetails
	{
		VkPhysicalDeviceFeatures Features{};
		VkPhysicalDeviceProperties Properties{};
		std::vector<VkExtensionProperties> Extensions;
		std::vector<VkQueueFamilyProperties> QueueFamilyProperties;
	};

	class VulkanPhysicalDevice
	{
	public:
		VulkanPhysicalDevice& operator=(VulkanPhysicalDevice&) = delete;

		void Init(VkPhysicalDevice physicalDevice);
		bool IsExtensionSupported(const char* deviceExtensionName);

		VkPhysicalDevice GetVulkanPhysicalDevice() const { return m_PhysicalDevice; }
		const PhysicalDeviceSupportDetails& GetSupportDetails() const { return m_SupportDetails; }
		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }

		bool IsFormatFeatureSupported(VkFormat format, VkImageTiling tiling, VkFormatFeatureFlags formatFeatures) const;
		
		VkFormat GetBestDepthOnlyFormat(VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
		VkFormat GetBestDepthStencilFormat(VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL) const;
		
		VkSampleCountFlagBits GetFramebufferMultisampleCount() const;
	private:
		void AquireSupportDetails();
		void SelectQueueFamilyIndices();
	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		QueueFamilyIndices m_QueueFamilyIndices;
		PhysicalDeviceSupportDetails m_SupportDetails;
	};

	class VulkanDevice
	{
	public:
		VulkanDevice& operator=(VulkanDevice&) = delete;
	public:
		void Init();
		void Shutdown();

		VkDevice GetVulkanDevice() const { return m_Device; }

		VkQueue GetQueue(QueueType queueType);
		VkCommandPool GetCommandPool(QueueType queueType, bool transientPool = false);

		VkCommandBuffer BeginCommandBuffer(QueueType queueType, bool oneTimeSubmit = true);
		void EndCommandBuffer();
	private:
		void CreateDevice();
		void CreateCommandPools();
		void CreateSyncronizationObjects();
	private:
		std::vector<const char*> BuildRequestedDeviceExtensions();
		void BuildEnabledFeatures(VkPhysicalDeviceFeatures* features);
	private:
		VkDevice m_Device = VK_NULL_HANDLE;

		VkQueue m_GraphicsQueue = VK_NULL_HANDLE;
		VkQueue m_TransferQueue = VK_NULL_HANDLE;
		VkQueue m_ComputeQueue = VK_NULL_HANDLE;
		VkQueue m_PresentQueue = VK_NULL_HANDLE;

		VkCommandPool m_GraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_TransferCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_ComputeCommandPool = VK_NULL_HANDLE;

		VkCommandPool m_TransientGraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_TransientTransferCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_TransientComputeCommandPool = VK_NULL_HANDLE;

		VkFence m_WaitFence = VK_NULL_HANDLE;

		VkQueue m_ActiveQueue = VK_NULL_HANDLE;
		VkCommandBuffer m_ActiveCommandBuffer = VK_NULL_HANDLE;
		VkCommandPool m_ActiveCommandPool = VK_NULL_HANDLE;
	};

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