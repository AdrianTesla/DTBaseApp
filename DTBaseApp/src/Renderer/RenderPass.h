#pragma once
#include "Framebuffer.h"
#include "Core/Math.h"
#include "Pipeline.h"
#include "DX11Buffers.h"

namespace DT
{
	struct RenderPassSpecification
	{
		Ref<Pipeline> Pipeline;
		Ref<Framebuffer> TargetFramebuffer;
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& specification);
		void SetInput(const std::string& name, const Ref<Image2D>& image, uint32 slot);
		void SetInput(const std::string& name, const Ref<UniformBuffer>& uniformBuffer, uint32 slot);

		Ref<Framebuffer> GetOutput() const;
		Ref<Image2D> GetInputImage(const std::string& name) const;
		Ref<UniformBuffer> GetInputUniformBuffer(const std::string& name) const;

		void Begin(bool explicitClear = true);
		void End();

		RenderPassSpecification& GetSpecification() { return m_Specification; }

		static Ref<RenderPass> Create(const RenderPassSpecification& specification) { return CreateRef<RenderPass>(specification); }
	private:
		std::unordered_map<std::string, std::pair<Ref<Image2D>, uint32>> m_InputImages;
		std::unordered_map<std::string, std::pair<Ref<UniformBuffer>, uint32>> m_InputUniformBuffers;
		RenderPassSpecification m_Specification;
	};
}
