#pragma once
#include "GraphicsContext.h"

namespace DT
{
	class Sampler
	{
	public:
		Sampler(bool bilinear);
		void Bind(uint32 slot);
	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_Sampler;
	};
}
