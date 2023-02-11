#include "RendererContext.h"
#include "Platform/API_Vulkan/VulkanContext.h"

namespace DT
{
    Ref<RendererContext> RendererContext::Create(const Ref<Window>& window)
    {
        return Ref<VulkanContext>::Create(window);
    }
}