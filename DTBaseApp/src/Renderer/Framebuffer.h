#pragma once
#include "GraphicsContext.h"

namespace DT
{
	struct FramebufferSpecification
	{
		bool SwapchainTarget = true;
	};

	class Framebuffer
	{
	public:
		Framebuffer(const FramebufferSpecification& specification);
		void Bind();
		void OnResize(int32 width, int32 height);
		ID3D11RenderTargetView* GetRTV() const { return m_RenderTargetView; }
	private:
		ID3D11RenderTargetView* m_RenderTargetView = nullptr;
		FramebufferSpecification m_Specification;
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
