#pragma once
#include "Vulkan.h"
#include "Renderer/RendererContext.h"

namespace DT
{
	struct QueueFamilyIndices
	{
		std::optional<uint32> GraphicsIndex;
		std::optional<uint32> TransferIndex;
		std::optional<uint32> ComputeIndex;

		constexpr bool IsComplete()
		{
			return GraphicsIndex.has_value() && TransferIndex.has_value() && ComputeIndex.has_value();
		}
	};

	struct VulkanPhysicalDevice
	{
	public:
		void Init(VkPhysicalDevice physicalDevice);
		bool IsExtensionSupported(const char* deviceExtensionName);
	private:
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_PhysicalDeviceProperties{};
		VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures{};
		std::vector<VkExtensionProperties> m_SupportedDeviceExtensions;
		std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties;
		QueueFamilyIndices m_QueueFamilyIndices;
	};

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
		void CreateLogicalDevice();
	private:
		std::vector<const char*> BuildRequestedInstanceExtensions();
		std::vector<const char*> BuildRequestedInstanceLayers();
	private:
		Ref<Window> m_Window;

		inline static VkInstance s_Instance = VK_NULL_HANDLE;

		inline static std::vector<VkLayerProperties> s_AvailableInstanceLayers;
		inline static std::vector<VkExtensionProperties> s_AvailableInstanceExtensions;

		VkDebugUtilsMessengerEXT m_DebugMessenger = VK_NULL_HANDLE;
		
		VulkanPhysicalDevice m_PhysicalDevice;
	};
}