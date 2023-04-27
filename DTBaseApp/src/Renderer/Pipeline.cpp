#include "Pipeline.h"
#include <d3dcompiler.h>
#pragma comment (lib,"d3dcompiler.lib")
#pragma comment (lib,"dxguid.lib")

namespace DT
{
	namespace Utils
	{
        static constexpr const char* s_ShaderFolder = "src/Renderer/Shaders/ShaderBinaries/";

        HRESULT CreateInputLayoutDescFromVertexShaderSignature(ID3DBlob* pShaderBlob, ID3D11Device* pD3DDevice, ID3D11InputLayout** pInputLayout)
        {
            // Reflect shader info
            ID3D11ShaderReflection* pVertexShaderReflection = NULL;
            if (FAILED(D3DReflect(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&pVertexShaderReflection)))
            {
                return S_FALSE;
            }

            // Get shader info
            D3D11_SHADER_DESC shaderDesc;
            pVertexShaderReflection->GetDesc(&shaderDesc);

            // Read input layout description from shader info
            std::vector<D3D11_INPUT_ELEMENT_DESC> inputLayoutDesc;
            for (uint32 i = 0; i < shaderDesc.InputParameters; i++)
            {
                D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
                pVertexShaderReflection->GetInputParameterDesc(i, &paramDesc);

                // fill out input element desc
                D3D11_INPUT_ELEMENT_DESC elementDesc;
                elementDesc.SemanticName = paramDesc.SemanticName;
                elementDesc.SemanticIndex = paramDesc.SemanticIndex;
                elementDesc.InputSlot = 0;
                elementDesc.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
                elementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
                elementDesc.InstanceDataStepRate = 0;

                // determine DXGI format
                if (paramDesc.Mask == 1)
                {
                    if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32_UINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32_SINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32_FLOAT;
                }
                else if (paramDesc.Mask <= 3)
                {
                    if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32_UINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32_SINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
                }
                else if (paramDesc.Mask <= 7)
                {
                    if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_UINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_SINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
                }
                else if (paramDesc.Mask <= 15)
                {
                    if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_UINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_UINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_SINT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_SINT;
                    else if (paramDesc.ComponentType == D3D_REGISTER_COMPONENT_FLOAT32) elementDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
                }

                //save element desc
                inputLayoutDesc.push_back(elementDesc);
            }

            // Try to create Input Layout
            HRESULT hr = pD3DDevice->CreateInputLayout(&inputLayoutDesc[0], (uint32)inputLayoutDesc.size(), pShaderBlob->GetBufferPointer(), (SIZE_T)pShaderBlob->GetBufferSize(), pInputLayout);


            //Free allocation shader reflection memory
            pVertexShaderReflection->Release();
            return hr;
        }
	}

	Pipeline::Pipeline(const PipelineSpecification& specification)
		: m_Specification(specification)
	{
	    //Load, create and set Vertex Shader
		ID3DBlob* vertexShaderBytecode;
		DXCALL(D3DReadFileToBlob((Utils::s_ShaderFolder / m_Specification.VertexShaderPath).c_str(), &vertexShaderBytecode));
		DXCALL(GraphicsContext::GetDevice()->CreateVertexShader(vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), nullptr, &m_VertexShader));

		//Load, create and set Pixel Shader
		ID3DBlob* pixelShaderBytecode;
		DXCALL(D3DReadFileToBlob((Utils::s_ShaderFolder / m_Specification.PixelShaderPath).c_str(), &pixelShaderBytecode));
		DXCALL(GraphicsContext::GetDevice()->CreatePixelShader(pixelShaderBytecode->GetBufferPointer(), pixelShaderBytecode->GetBufferSize(), nullptr, &m_PixelShader));
		pixelShaderBytecode->Release();

		//Create Input Layout and validate with Vertex Shader
        DXCALL(Utils::CreateInputLayoutDescFromVertexShaderSignature(
            vertexShaderBytecode,
            GraphicsContext::GetDevice(),
            &m_InputLayout));
		
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