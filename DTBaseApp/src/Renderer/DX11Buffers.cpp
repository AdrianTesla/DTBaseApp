#include "DX11Buffers.h"

namespace DT
{
	VertexBuffer::VertexBuffer(const void* vertices, uint32 size)
	{
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = size;
		vertexBufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
		vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescriptor.CPUAccessFlags = 0u;
		vertexBufferDescriptor.MiscFlags = 0u;
		vertexBufferDescriptor.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA vertexData{};
		vertexData.pSysMem = vertices;
		DXCALL(GraphicsContext::GetDevice()->CreateBuffer(&vertexBufferDescriptor, &vertexData, &m_VertexBuffer));
	}

	void VertexBuffer::Bind(uint32 stride)
	{
		UINT offsets = 0u;
		GraphicsContext::GetContext()->IASetVertexBuffers(0u, 1u, m_VertexBuffer.GetAddressOf(), &stride, &offsets);
	}
}