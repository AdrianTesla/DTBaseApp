#pragma once
#include "GraphicsContext.h"

namespace DT
{
	struct PipelineSpecification
	{
		std::filesystem::path VertexShaderPath;
		std::filesystem::path PixelShaderPath;
		bool BlendingEnabled = false;
	};

	class Pipeline
	{
	public:
		Pipeline(const PipelineSpecification& specification);
		void Bind();
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_BlendState;

		PipelineSpecification m_Specification;
	};
}
