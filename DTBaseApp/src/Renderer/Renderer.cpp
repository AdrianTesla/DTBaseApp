#include "Renderer.h"
#include "Platform/API_Vulkan/VulkanContext.h"

namespace DT
{
    uint32 Renderer::CurrentFrame()
    {
        return VulkanContext::CurrentFrame();
    }
}