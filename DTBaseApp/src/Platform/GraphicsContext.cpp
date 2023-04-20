#include "Core/Application.h"
#include "GraphicsContext.h"
#include <d3dcompiler.h>
#pragma comment (lib,"d3d11.lib")
#pragma comment (lib,"d3dcompiler.lib")

namespace DT
{
	GraphicsContext::GraphicsContext(const Window* window)
	{
		m_Window = window;
	}

	void GraphicsContext::Init()
	{
		DXGI_SWAP_CHAIN_DESC swapchainDescriptor{};
		swapchainDescriptor.BufferCount = 2u;
		swapchainDescriptor.BufferDesc.Width = 0u;
		swapchainDescriptor.BufferDesc.Height = 0u;
		swapchainDescriptor.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapchainDescriptor.BufferDesc.RefreshRate.Denominator = 0u;
		swapchainDescriptor.BufferDesc.RefreshRate.Numerator = 0u;
		swapchainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapchainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapchainDescriptor.Windowed = TRUE;
		swapchainDescriptor.SampleDesc.Count = 1u;
		swapchainDescriptor.SampleDesc.Quality = 0u;
		swapchainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapchainDescriptor.Flags = 0u;
		swapchainDescriptor.OutputWindow = (HWND)m_Window->GetNativeWindow();

		UINT deviceCreateFlags = 0u;
#ifdef DT_DEBUG
		deviceCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif // DT_DEBUG

		DXCALL(D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			deviceCreateFlags,
			nullptr,
			0u,
			D3D11_SDK_VERSION,
			&swapchainDescriptor,
			&m_Swapchain,
			&m_Device,
			nullptr,
			&m_Context
		));
		
		ID3D11Texture2D* pBackBuffer;
		DXCALL(m_Swapchain->GetBuffer(0u, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer));
		DXCALL(m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_RenderTargetView));
	}

	void GraphicsContext::Present()
	{
		DXCALL(m_Swapchain->Present(1u, 0u));
	}

	void GraphicsContext::BeginFrame()
	{
		float windowColor[4];
		windowColor[0] = 0.0f; //R
		windowColor[1] = 0.0f; //G
		windowColor[2] = 0.0f; //B
		windowColor[3] = 1.0f; //A
		m_Context->ClearRenderTargetView(m_RenderTargetView.Get(), windowColor);
	}

	void GraphicsContext::DrawTriangle()
	{
		m_Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//Set Viewport
		D3D11_VIEWPORT viewport{};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (float)Application::Get().GetWindow().GetWidth();
		viewport.Height = (float)Application::Get().GetWindow().GetHeight();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_Context->RSSetViewports(1u, &viewport);

		//Create Triangle Vertices 
		struct Vertex
		{
			float x;
			float y;
		};

		Vertex vertices[3];
		vertices[0] = { 0.0f,0.5f };
		vertices[1] = { 0.3f,-0.4f };
		vertices[2] = { -0.3f,-0.4f };

		D3D11_INPUT_ELEMENT_DESC inputElementDescriptor{};
		inputElementDescriptor.AlignedByteOffset = 0u;
		inputElementDescriptor.Format = DXGI_FORMAT_R32G32_FLOAT;
		inputElementDescriptor.InputSlot = 0u;
		inputElementDescriptor.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputElementDescriptor.InstanceDataStepRate = 0u;
		inputElementDescriptor.SemanticIndex = 0u;
		inputElementDescriptor.SemanticName = "VertexPosition";

		
		//Upload vertices on GPU
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = sizeof(vertices);
		vertexBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescriptor.CPUAccessFlags = 0u;
		vertexBufferDescriptor.MiscFlags = 0u;
		vertexBufferDescriptor.StructureByteStride = sizeof(Vertex);

		D3D11_SUBRESOURCE_DATA vertexData{};
		vertexData.pSysMem = vertices;
		DXCALL(m_Device->CreateBuffer(&vertexBufferDescriptor, &vertexData, &m_VertexBuffer));

		UINT strides = sizeof(Vertex);
		UINT offsets = 0u;
		m_Context->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), &strides, &offsets);

		//Load, create and set Vertex Shader
		ID3DBlob* vertexShaderBytecode;
		DXCALL(D3DReadFileToBlob(L"src/Renderer/Shaders/ShaderBinaries/TriangleVS.cso", &vertexShaderBytecode));
		DXCALL(m_Device->CreateVertexShader(vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), nullptr, &m_VertexShader));
		m_Context->VSSetShader(m_VertexShader.Get(), nullptr, 0u);

		//Load, create and set Pixel Shader
		ID3DBlob* pixelShaderBytecode;
		DXCALL(D3DReadFileToBlob(L"src/Renderer/Shaders/ShaderBinaries/TrianglePS.cso", &pixelShaderBytecode));
		DXCALL(m_Device->CreatePixelShader(pixelShaderBytecode->GetBufferPointer(), pixelShaderBytecode->GetBufferSize(), nullptr, &m_PixelShader));
		m_Context->PSSetShader(m_PixelShader.Get(), nullptr, 0u);
		pixelShaderBytecode->Release();

		m_Context->OMSetRenderTargets(1u, m_RenderTargetView.GetAddressOf(), nullptr);

		//Create Input Layout and validate with Vertex Shader 
		m_Device->CreateInputLayout(&inputElementDescriptor, 1u, vertexShaderBytecode->GetBufferPointer(), vertexShaderBytecode->GetBufferSize(), &m_InputLayout);
		vertexShaderBytecode->Release();
		m_Context->IASetInputLayout(m_InputLayout.Get());

		//Draw Triangle
		m_Context->Draw(3u, 0u);
	}
}
