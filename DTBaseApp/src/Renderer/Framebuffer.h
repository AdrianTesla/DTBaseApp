#pragma once
#include "GraphicsContext.h"
#include "Texture.h"

namespace DT
{
	struct FramebufferSpecification
	{
		bool SwapchainTarget = true;
		ImageFormat Format = ImageFormat::RGBA8;
		uint32 Width = 0u;
		uint32 Height = 0u;
		float Scale = 1.0f;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& specification);
		void Bind();
		void OnResize(int32 width, int32 height);
		ID3D11RenderTargetView* GetColorAttachment() const;
	private:
		ID3D11RenderTargetView* m_RenderTargetView = nullptr;
		FramebufferSpecification m_Specification;
		Ref<Image2D> m_ColorAttachment;
	};

	class FramebufferPool
	{
	public:
		static void AddFramebuffer(Framebuffer* framebuffer)
		{
			s_Framebuffers.emplace_back(framebuffer);
		}
		static void OnResize(int32 width, int32 height)
		{
			for (auto& framebuffer : s_Framebuffers)
			{
				framebuffer->OnResize(width, height);
			}
		}
	private:
		inline static std::vector<Framebuffer*> s_Framebuffers;
	};
}
