#pragma once
#include "GraphicsContext.h"

namespace DT
{
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
