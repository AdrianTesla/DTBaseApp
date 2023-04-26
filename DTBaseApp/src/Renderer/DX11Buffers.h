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
}
