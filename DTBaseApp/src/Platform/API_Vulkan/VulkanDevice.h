#pragma once
#include "Vulkan.h"
#include <optional>

namespace DT
{
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
		void Init();
		void Shutdown();

		VkCommandBuffer AllocateGraphicsCommandBuffer();
		VkDevice GetVulkanDevice() const { return m_Device; }

		VkQueue GetPresentQueue() const { return m_PresentQueue; }
	private:
		void CreateDevice();
		void CreateCommandPools();
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
	};
}