#pragma once
#include "Renderer/GraphicsContext.h"

namespace DT
{
	class VertexBuffer
	{
	public:
		VertexBuffer(const void* vertices, uint64 size);
		void Bind(uint32 stride);
	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	};
}
