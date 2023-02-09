#include "RendererContext.h"
#include "Platform/API_Vulkan/VulkanContext.h"

namespace DT
{
    Ref<RendererContext> RendererContext::Create()
    {
        return Ref<VulkanContext>::Create();
    }
}