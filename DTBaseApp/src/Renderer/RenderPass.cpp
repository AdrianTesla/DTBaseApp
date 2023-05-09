#include "RenderPass.h"

namespace DT
{
	RenderPass::RenderPass(const RenderPassSpecification& specification)
		: m_Specification(specification)
	{
	}

	void RenderPass::SetInput(const std::string& name, const Ref<Image2D>& image, uint32 slot)
	{
		m_InputImages[name] = { image, slot };
	}

	void RenderPass::SetInput(const std::string& name, const Ref<UniformBuffer>& uniformBuffer, uint32 slot)
	{
		m_InputUniformBuffers[name] = { uniformBuffer, slot };
	}

	Ref<Framebuffer> RenderPass::GetOutput() const
	{
		return m_Specification.TargetFramebuffer;
	}

	Ref<Image2D> RenderPass::GetInputImage(const std::string& name) const
	{
		ASSERT(m_InputImages.contains(name));
		return m_InputImages.at(name).first;
	}

	Ref<UniformBuffer> RenderPass::GetInputUniformBuffer(const std::string& name) const
	{
		ASSERT(m_InputUniformBuffers.contains(name));
		return m_InputUniformBuffers.at(name).first;
	}

	void RenderPass::Begin(bool explicitClear)
	{
		if(m_Specification.Pipeline)
			m_Specification.Pipeline->Bind();

		if (explicitClear)
			m_Specification.TargetFramebuffer->ClearAttachment(m_Specification.ClearColor);

		ID3D11ShaderResourceView* nullRTV = nullptr;
		GraphicsContext::GetContext()->PSSetShaderResources(0u, 1u, &nullRTV);
		m_Specification.TargetFramebuffer->Bind();

		for (auto&& [name, image] : m_InputImages)
		{
			ID3D11ShaderResourceView* srv = image.first->GetSRV();
			GraphicsContext::GetContext()->PSSetShaderResources(image.second, 1u, &srv);
		}

		for (auto&& [name, uniformBuffer] : m_InputUniformBuffers)
		{
			uniformBuffer.first->BindVS(uniformBuffer.second);
			uniformBuffer.first->BindPS(uniformBuffer.second);
		}
	}

	void RenderPass::End()
	{
	}
}