#pragma once
#include "GraphicsContext.h"

namespace DT
{
	enum class ImageFormat
	{
		RGBA8,
		RGBA16F,
		R11G11B10F
	};

	struct ImageSpecification
	{
		uint32 Width;
		uint32 Height;
		ImageFormat Format = ImageFormat::RGBA8;
	};

	class Image2D
	{
	public:
		Image2D(const ImageSpecification& specification);
	private:
		Microsoft::WRL::ComPtr<ID3D11Texture2D> m_Image;
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
}
