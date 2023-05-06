#pragma once
#include "GraphicsContext.h"
#include "Texture.h"
#include "Core/Math.h"

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
		void Resize(int32 width, int32 height, bool force = false);
		void ClearAttachment(const glm::vec4& color);
		Ref<Image2D> GetImage() const { return m_ColorImage; }
		const FramebufferSpecification& GetSpecification() const { return m_Specification; }
	private:
		uint32 m_Width = 0u;
		uint32 m_Height = 0u;
		Ref<Image2D> m_ColorImage;
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
				if (framebuffer->GetSpecification().Width == 0u || framebuffer->GetSpecification().Height == 0u)
					framebuffer->Resize(width, height);
			}
		}
	private:
		inline static std::vector<Framebuffer*> s_Framebuffers;
	};
}
