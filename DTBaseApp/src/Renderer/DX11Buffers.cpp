#include "DX11Buffers.h"

namespace DT
{
	VertexBuffer::VertexBuffer(uint64 size)
	{
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = (uint32)size;
		vertexBufferDescriptor.Usage = D3D11_USAGE_DYNAMIC;
		vertexBufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vertexBufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vertexBufferDescriptor.MiscFlags = 0u;
		vertexBufferDescriptor.StructureByteStride = 0u;
		DXCALL(GraphicsContext::GetDevice()->CreateBuffer(&vertexBufferDescriptor, nullptr, &m_VertexBuffer));
	}

	VertexBuffer::VertexBuffer(const void* vertices, uint64 size)
	{
		D3D11_BUFFER_DESC vertexBufferDescriptor{};
		vertexBufferDescriptor.ByteWidth = (uint32)size;
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

	void VertexBuffer::SetData(const void* vertices, uint64 size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedMemory{};
		DXCALL(GraphicsContext::GetContext()->Map(m_VertexBuffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &mappedMemory));
		memcpy(mappedMemory.pData, vertices, size);
		GraphicsContext::GetContext()->Unmap(m_VertexBuffer.Get(), 0u);
	}
}