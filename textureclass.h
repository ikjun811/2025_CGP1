////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _TEXTURECLASS_H_
#define _TEXTURECLASS_H_


//////////////
// INCLUDES //
//////////////
#include <d3d11.h>
#include <wincodec.h>

////////////////////////////////////////////////////////////////////////////////
// Class name: TextureClass
////////////////////////////////////////////////////////////////////////////////
class TextureClass
{
public:
	TextureClass();
	TextureClass(const TextureClass&);
	~TextureClass();

	bool Initialize(ID3D11Device* device, const WCHAR* filename);
	bool Initialize(ID3D11Device* device, const void* pData, size_t dataSize);

	void Shutdown();

	ID3D11ShaderResourceView* GetTexture();


private:
	ID3D11ShaderResourceView* m_texture;
};

#endif