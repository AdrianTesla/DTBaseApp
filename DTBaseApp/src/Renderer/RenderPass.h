#pragma once
#include "Framebuffer.h"
#include "Core/Math.h"

namespace DT
{
	struct RenderPassSpecification
	{
		Ref<Framebuffer> TargetFrameBuffer;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& specification);
		const RenderPassSpecification& GetSpecification() const { return m_Specification; }
	private:
		RenderPassSpecification m_Specification;
	};
}
