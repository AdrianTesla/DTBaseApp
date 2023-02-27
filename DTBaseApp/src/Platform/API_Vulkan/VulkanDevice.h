#pragma once
#include "Vulkan.h"
#include <optional>

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
		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_QueueFamilyIndices; }
		const VkPhysicalDeviceFeatures& GetSupportedFeatures() const { return m_SupportDetails.Features; }
	private:
		void GetSupportDetails();
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
		VkCommandPool GetCommandPool(QueueType queueType, bool stagingPool = false);

		//VkCommandBuffer AllocateCommandBuffer(QueueType queueType, bool stagingPool = false, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

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

		VkCommandPool m_StagingGraphicsCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_StagingTransferCommandPool = VK_NULL_HANDLE;
		VkCommandPool m_StagingComputeCommandPool = VK_NULL_HANDLE;

		VkFence m_WaitFence = VK_NULL_HANDLE;

		VkQueue m_ActiveQueue = VK_NULL_HANDLE;
		VkCommandBuffer m_ActiveCommandBuffer = VK_NULL_HANDLE;
		VkCommandPool m_ActiveCommandPool = VK_NULL_HANDLE;
	};
}