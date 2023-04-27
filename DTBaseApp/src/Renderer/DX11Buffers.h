#pragma once
#include "Renderer/GraphicsContext.h"

namespace DT
{
	class VertexBuffer
	{
	public:
		VertexBuffer(uint64 size);
		VertexBuffer(const void* vertices, uint64 size);

		void Bind(uint32 stride);
		void SetData(const void* vertices, uint64 size);
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	};

	class UniformBuffer
	{
	public:
		UniformBuffer(uint64 size);

		void BindVS(uint32 slot);
		void BindPS(uint32 slot);
		void SetData(const void* data, uint64 size);
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_UniformBuffer;
	};
}
