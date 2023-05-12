#pragma once
#include "GraphicsContext.h"

namespace DT
{
	enum class BlendingMode
	{
		None,
		Alpha,
		Additive
	};

	struct PipelineSpecification
	{
		std::filesystem::path VertexShaderPath;
		std::filesystem::path PixelShaderPath;
		BlendingMode BlendingMode = BlendingMode::None;
	};

	class Pipeline
	{
	public:
		Pipeline(const PipelineSpecification& specification);
		void Bind();

		static Ref<Pipeline> Create(const PipelineSpecification& specification) { return CreateRef<Pipeline>(specification); }
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;

		PipelineSpecification m_Specification;
	};
}
