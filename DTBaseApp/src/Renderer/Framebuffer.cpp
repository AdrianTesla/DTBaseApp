#include "Framebuffer.h"
#include "Core/Application.h"

namespace DT
{
	Framebuffer::Framebuffer(const FramebufferSpecification& specification)
		: m_Specification(specification)
	{
		FramebufferPool::AddFramebuffer(this);

		if (specification.SwapchainTarget)
		{
			m_RenderTargetView = GraphicsContext::GetSwapchainRTV();
		}
		else
		{
			ImageSpecification imageSpecification;
			imageSpecification.Format = specification.Format;
			if (specification.Width == 0u || specification.Height == 0u)
			{
				imageSpecification.Width = (uint32)((float)Application::Get().GetWindow().GetWidth() * specification.Scale);
				imageSpecification.Height = (uint32)((float)Application::Get().GetWindow().GetHeight() * specification.Scale);
			}
			else
			{
				imageSpecification.Width = specification.Width;
				imageSpecification.Height = specification.Height;
			}

			imageSpecification.Usage = ImageUsage::Attachment;
			m_ColorAttachment = CreateRef<Image2D>(imageSpecification);

			D3D11_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
			rtvDescriptor.Format = Utils::ToDXGIFormat(imageSpecification.Format);
			rtvDescriptor.Texture2D.MipSlice = 0u;
			rtvDescriptor.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			DXCALL(GraphicsContext::GetDevice()->CreateRenderTargetView(m_ColorAttachment->GetResource(), &rtvDescriptor, &m_RenderTargetView));
		}
	}

	void Framebuffer::Bind()
	{
		GraphicsContext::GetContext()->OMSetRenderTargets(1u, &m_RenderTargetView, nullptr);
	}

	void Framebuffer::OnResize(int32 width, int32 height)
	{
		if (m_Specification.SwapchainTarget)
			m_RenderTargetView = GraphicsContext::GetSwapchainRTV();
		else
		{
			//TODO!
		}
	}

	ID3D11RenderTargetView* Framebuffer::GetColorAttachment() const
	{
		if (m_Specification.SwapchainTarget)
			return GraphicsContext::GetSwapchainRTV();
		else
			return m_RenderTargetView;
	}
}