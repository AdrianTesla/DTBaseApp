#include "Texture.h"
#include <stb_image.h>

namespace DT
{
	Image2D::Image2D(const ImageSpecification& specification)
		: m_Specification(specification)
	{
		D3D11_TEXTURE2D_DESC textureDescriptor{};
		textureDescriptor.Width = specification.Width;
		textureDescriptor.Height = specification.Height;
		textureDescriptor.MipLevels = 1u;
		textureDescriptor.ArraySize = 1u;
		textureDescriptor.Format = Utils::ToDXGIFormat(specification.Format);
		textureDescriptor.SampleDesc.Count = 1u;
		textureDescriptor.SampleDesc.Quality = 0u;
		textureDescriptor.Usage = D3D11_USAGE_DEFAULT;
		textureDescriptor.CPUAccessFlags = 0u;
		textureDescriptor.MiscFlags = 0u;

		if(specification.Usage == ImageUsage::Texture)
			textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		else
			textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		
		DXCALL(GraphicsContext::GetDevice()->CreateTexture2D(&textureDescriptor, nullptr, &m_Image));
	}

	Texture2D::Texture2D(const std::filesystem::path& filepath)
	{
		int32 width;
		int32 height;
		stbi_uc* pixels = stbi_load(filepath.string().c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		D3D11_TEXTURE2D_DESC textureDescriptor{};
		textureDescriptor.Width = (uint32)width;
		textureDescriptor.Height = (uint32)height;
		textureDescriptor.MipLevels = 1u;
		textureDescriptor.ArraySize = 1u;
		textureDescriptor.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDescriptor.SampleDesc.Count = 1u;
		textureDescriptor.SampleDesc.Quality = 0u;
		textureDescriptor.Usage = D3D11_USAGE_DEFAULT;
		textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescriptor.CPUAccessFlags = 0u;
		textureDescriptor.MiscFlags = 0u;

		D3D11_SUBRESOURCE_DATA initialData{};
		initialData.pSysMem = pixels;
		initialData.SysMemPitch = (uint32)(width * 4);
		initialData.SysMemSlicePitch = 0u;
		DXCALL(GraphicsContext::GetDevice()->CreateTexture2D(&textureDescriptor, &initialData, &m_Texture));

		D3D11_SHADER_RESOURCE_VIEW_DESC viewDescriptor{};
		viewDescriptor.Format = textureDescriptor.Format;
		viewDescriptor.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		viewDescriptor.Texture2D.MipLevels = 1u;
		viewDescriptor.Texture2D.MostDetailedMip = 0u;
		GraphicsContext::GetDevice()->CreateShaderResourceView(m_Texture.Get(), &viewDescriptor, &m_ShaderResourceView);
	}

	void Texture2D::Bind(uint32 slot)
	{
		GraphicsContext::GetContext()->PSSetShaderResources(slot, 1u, m_ShaderResourceView.GetAddressOf());
	}

	bool Texture2D::Compare(const Ref<Texture2D>& texture2D) const
	{
		return m_ShaderResourceView == texture2D->m_ShaderResourceView;
	}

}