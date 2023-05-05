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
		glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
	};

	class RenderPass
	{
	public:
		RenderPass(const RenderPassSpecification& specification);
		void SetInput(const std::string& name, const Ref<Image2D>& image, uint32 slot);
		void SetInput(const std::string& name, const Ref<UniformBuffer>& uniformBuffer, uint32 slot);
		void SetOutput(const Ref<Framebuffer>& framebuffer);

		Ref<Framebuffer> GetOutput() const;
		Ref<Image2D> GetInputImage(const std::string& name) const;
		Ref<UniformBuffer> GetInputUniformBuffer(const std::string& name) const;

		void Begin(bool explicitClear = true);
		void End();

		RenderPassSpecification& GetSpecification() { return m_Specification; }
	private:
		std::unordered_map<std::string, std::pair<Ref<Image2D>, uint32>> m_InputImages;
		std::unordered_map<std::string, std::pair<Ref<UniformBuffer>, uint32>> m_InputUniformBuffers;
		Ref<Framebuffer> m_TargetFramebuffer;
		RenderPassSpecification m_Specification;
	};
}
