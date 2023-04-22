#pragma once
#include "GraphicsContext.h"

namespace DT
{
	class Pipeline
	{
	public:
		Pipeline();
		void Bind();
	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_VertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_PixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
	};
}
