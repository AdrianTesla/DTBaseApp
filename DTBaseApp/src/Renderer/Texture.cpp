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
		textureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		if(specification.Usage == ImageUsage::Attachment)
			textureDescriptor.BindFlags |= D3D11_BIND_RENDER_TARGET;
		
		DXCALL(GraphicsContext::GetDevice()->CreateTexture2D(&textureDescriptor, nullptr, &m_Image));

		//Create shader resource view 
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDescriptor{};
		srvDescriptor.Format = textureDescriptor.Format;
		srvDescriptor.Texture2D.MipLevels = 1u;
		srvDescriptor.Texture2D.MostDetailedMip = 0u;
		srvDescriptor.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		DXCALL(GraphicsContext::GetDevice()->CreateShaderResourceView(m_Image.Get(), &srvDescriptor, &m_ShaderResourceView));

		//Create Render target view 
		if (specification.Usage == ImageUsage::Attachment)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDescriptor{};
			rtvDescriptor.Format = textureDescriptor.Format;
			rtvDescriptor.Texture2D.MipSlice = 0u;
			rtvDescriptor.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			DXCALL(GraphicsContext::GetDevice()->CreateRenderTargetView(m_Image.Get(), &rtvDescriptor, &m_RenderTargetView));
		}
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