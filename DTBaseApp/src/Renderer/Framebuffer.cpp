#include "Framebuffer.h"
#include "Core/Application.h"

namespace DT
{
	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		FramebufferPool::AddFramebuffer(this);

		if (specification.SwapchainTarget)
			return;

		if (specification.Width == 0u || specification.Height == 0u)
		{
			int32 windowWidth = Application::Get().GetWindow().GetWidth();
			int32 windowHeight = Application::Get().GetWindow().GetHeight();
			Resize((int32)(windowWidth * specification.Scale), (int32)(windowHeight * specification.Scale));
		}
		else
			Resize((int32)(specification.Width * specification.Scale), (int32)(specification.Height * specification.Scale));
	}

	void Framebuffer::Bind()
	{
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)m_Width;
		viewport.Height = (float)m_Height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		GraphicsContext::GetContext()->RSSetViewports(1u, &viewport);

		ID3D11RenderTargetView* rtv;
		if (m_Specification.SwapchainTarget)
			rtv = GraphicsContext::GetSwapchainRTV();
		else
			rtv = m_ColorAttachment->GetRTV();

		GraphicsContext::GetContext()->OMSetRenderTargets(1u, &rtv, nullptr);
	}

	void Framebuffer::Resize(int32 width, int32 height, bool force)
	{
		if (m_Specification.SwapchainTarget)
		{
			m_Width = width;
			m_Height = height;
			return;
		}

		uint32 newWidth = (uint32)(width * m_Specification.Scale);
		uint32 newHeight = (uint32)(width * m_Specification.Scale);
		if (!force && newWidth == m_Width && newHeight == m_Height)
			return;

		m_Width = newWidth;
		m_Height = newHeight;
		ImageSpecification imageSpecification{};
		imageSpecification.Format = m_Specification.Format;
		imageSpecification.Width = m_Width;
		imageSpecification.Height = m_Height;
		imageSpecification.Usage = ImageUsage::Attachment;
		m_ColorAttachment = CreateRef<Image2D>(imageSpecification);
	}

	void Framebuffer::ClearAttachment(const glm::vec4& color)
	{
		GraphicsContext::GetContext()->ClearRenderTargetView(GetColorAttachment(), glm::value_ptr(color));
	}

	ID3D11RenderTargetView* Framebuffer::GetColorAttachment() const
	{
		if (m_Specification.SwapchainTarget)
			return GraphicsContext::GetSwapchainRTV();
		else
			return m_ColorAttachment->GetRTV();
	}
}