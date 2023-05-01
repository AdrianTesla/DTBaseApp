#include "Sampler.h"

namespace DT
{
	Sampler::Sampler(bool bilinear)
	{
		D3D11_SAMPLER_DESC samplerDescriptor{};
		samplerDescriptor.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDescriptor.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samplerDescriptor.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		//samplerDescriptor.BorderColor = nullptr;
		samplerDescriptor.ComparisonFunc = D3D11_COMPARISON_LESS;
		samplerDescriptor.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samplerDescriptor.MaxAnisotropy = 0u;
		samplerDescriptor.MaxLOD = 100.0f;
		samplerDescriptor.MinLOD = 0.0f;
		samplerDescriptor.MipLODBias = 0.0f;
		DXCALL(GraphicsContext::GetDevice()->CreateSamplerState(&samplerDescriptor, &m_Sampler));
	}

	void Sampler::Bind(uint32 slot)
	{
		GraphicsContext::GetContext()->PSSetSamplers(slot, 1u, m_Sampler.GetAddressOf());
	}
}