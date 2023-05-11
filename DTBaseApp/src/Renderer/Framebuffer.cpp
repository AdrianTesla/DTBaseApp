#include "Framebuffer.h"
#include "Core/Application.h"

namespace DT
{
	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		FramebufferPool::AddFramebuffer(this);
		int32 width;
		int32 height;
		if (specification.Width == 0u || specification.Height == 0u)
		{
			width = Application::Get().GetWindow().GetWidth();
			height = Application::Get().GetWindow().GetHeight();
		}
		else
		{
			width = (uint32)(m_Specification.Width);
			height = (uint32)(m_Specification.Height);
		}

		ImageSpecification imageSpecification{};
		imageSpecification.Format = m_Specification.Format;
		imageSpecification.Width = width;
		imageSpecification.Height = height;
		imageSpecification.Usage = ImageUsage::Attachment;
		m_ColorImage = CreateRef<Image2D>(imageSpecification);

		Resize(width, height);
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
			rtv = m_ColorImage->GetRTV();

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
		uint32 newHeight = (uint32)(height * m_Specification.Scale);
		if (!force && newWidth == m_Width && newHeight == m_Height)
			return;

		m_Width = newWidth;
		m_Height = newHeight;

		m_ColorImage->Resize(m_Width, m_Height);
	}

	void Framebuffer::ClearAttachment(const glm::vec4& color)
	{
		ID3D11RenderTargetView* rtv;
		if (m_Specification.SwapchainTarget)
			rtv = GraphicsContext::GetSwapchainRTV();
		else
			rtv = m_ColorImage->GetRTV();

		GraphicsContext::GetContext()->ClearRenderTargetView(rtv, glm::value_ptr(color));
	}
}