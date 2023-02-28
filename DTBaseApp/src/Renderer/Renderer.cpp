#include "Renderer.h"
#include "Platform/API_Vulkan/VulkanContext.h"
#include "Platform/API_Vulkan/VulkanRenderer.h"

namespace DT
{
    static RendererBackend* s_RendererBackend = nullptr;

    void Renderer::Init()
    {
        s_RendererBackend = new VulkanRenderer();
        s_RendererBackend->Init();
    }

    void Renderer::Shutdown()
    {
        s_RendererBackend->Shutdown();
    }

    void Renderer::BeginFrame()
    {
        s_RendererBackend->BeginFrame();
    }

    void Renderer::EndFrame()
    {
        s_RendererBackend->EndFrame();
    }

    void Renderer::OnWindowResize()
    {
        s_RendererBackend->OnWindowResize();
    }

    void Renderer::SetVerticalSync(bool enabled)
    {
        s_RendererBackend->SetVerticalSync(enabled);
    }

    uint32 Renderer::CurrentFrame()
    {
        return s_RendererBackend->CurrentFrame();
    }
}