#include "VulkanSwapchain.h"
#include <GLFW/glfw3.h>
#include "VulkanContext.h"

namespace DT
{
	void VulkanSwapchain::Init(const Ref<Window>& window)
	{
		VkInstance instance = VulkanContext::Get().GetVulkanInstance();
		GLFWwindow* glfwWindow = (GLFWwindow*)window->GetPlatformWindow();
		VK_CALL(glfwCreateWindowSurface(instance, glfwWindow, nullptr, &m_Surface));
	}

	void VulkanSwapchain::Shutdown()
	{
		VkInstance instance = VulkanContext::Get().GetVulkanInstance();
		vkDestroySurfaceKHR(instance, m_Surface, nullptr);
		m_Surface = VK_NULL_HANDLE;
	}

	void VulkanSwapchain::Resize(int32 width, int32 height)
	{
	}

	void VulkanSwapchain::Present()
	{
	}
}
