#pragma once
#include "GraphicsContext.h"

namespace DT
{
	enum class ImageUsage
	{
		Texture,
		Attachment
	};

	enum class ImageFormat
	{
		RGBA8,
		RGBA16F,
		R11G11B10F
	};

	struct ImageSpecification
	{
		uint32 Width = 0u;
		uint32 Height = 0u;
		ImageFormat Format = ImageFormat::RGBA8;
		ImageUsage Usage = ImageUsage::Texture;
	};

	class Image2D
	{
	public:
		Image2D(const ImageSpecification& specification);
		ID3D11Texture2D* GetResource() const { return m_Image.Get(); }
		ID3D11RenderTargetView* GetRTV() const { return m_RenderTargetView.Get(); }
		ID3D11ShaderResourceView* GetSRV() const { return m_ShaderResourceView.Get(); }
		const ImageSpecification& GetSpecification() const { return m_Specification; }
		void Resize(uint32 width, uint32 height);
		uint32 GetWidth() const { return m_Width; }
		uint32 GetHeight() const { return m_Height; }
		void Invalidate();
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Image;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_RenderTargetView;
		uint32 m_Width = 0u;
		uint32 m_Height = 0u;
		ImageSpecification m_Specification;
	};

	class Texture2D
	{
	public:
		Texture2D(const std::filesystem::path& filepath);
		void Bind(uint32 slot);
		bool Compare(const Ref<Texture2D>& texture2D) const;
		ID3D11ShaderResourceView* GetShaderResourceView() const { return m_ShaderResourceView.Get(); }
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Texture;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_ShaderResourceView;
	};

	namespace Utils
	{
		inline DXGI_FORMAT ToDXGIFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGBA8:
				return DXGI_FORMAT_R8G8B8A8_UNORM;
			case ImageFormat::RGBA16F:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case ImageFormat::R11G11B10F:
				return DXGI_FORMAT_R11G11B10_FLOAT;
			}
			ASSERT(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}
}
