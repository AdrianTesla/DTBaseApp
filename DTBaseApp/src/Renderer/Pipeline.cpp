#include "Pipeline.h"
#include <d3dcompiler.h>
#pragma comment (lib,"d3dcompiler.lib")

namespace DT
{
	Pipeline::Pipeline()
	{
		D3D11_INPUT_ELEMENT_DESC inputElementDescriptor[2];
		inputElementDescriptor[0].AlignedByteOffset = 0u;
		inputElementDescriptor[0].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementDescriptor[0].InputSlot = 0u;
		inputElementDescriptor[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElementDescriptor[0].InstanceDataStepRate = 0u;
		inputElementDescriptor[0].SemanticIndex = 0u;
		inputElementDescriptor[0].SemanticName = "VertexPosition";

		inputElementDescriptor[1].AlignedByteOffset = 8u;
		inputElementDescriptor[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		inputElementDescriptor[1].InputSlot = 0u;
		inputElementDescriptor[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElementDescriptor[1].InstanceDataStepRate = 0u;
		inputElementDescriptor[1].SemanticIndex = 0u;
		inputElementDescriptor[1].SemanticName = "VertexColor";

		//Load, create and set Vertex Shader
		ID3DBlob* vertexShaderBytecode;
		DXCALL(D3DReadFileToBlob(L"src/Renderer/Shaders/ShaderBinaries/TriangleVS.cso", &vertexShaderBytecode));
		DXCALL(GraphicsContext::GetDevice()->CreateVertexShader(vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), nullptr, &m_VertexShader));

		//Load, create and set Pixel Shader
		ID3DBlob* pixelShaderBytecode;
		DXCALL(D3DReadFileToBlob(L"src/Renderer/Shaders/ShaderBinaries/TrianglePS.cso", &pixelShaderBytecode));
		DXCALL(GraphicsContext::GetDevice()->CreatePixelShader(pixelShaderBytecode->GetBufferPointer(), pixelShaderBytecode->GetBufferSize(), nullptr, &m_PixelShader));
		pixelShaderBytecode->Release();

		//Create Input Layout and validate with Vertex Shader 
		GraphicsContext::GetDevice()->CreateInputLayout(inputElementDescriptor, 2u, vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &m_InputLayout);
		vertexShaderBytecode->Release();
	}

	void Pipeline::Bind()
	{
		GraphicsContext::GetContext()->VSSetShader(m_VertexShader.Get(), nullptr, 0u);
		GraphicsContext::GetContext()->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		GraphicsContext::GetContext()->IASetInputLayout(m_InputLayout.Get());
		GraphicsContext::GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}
}