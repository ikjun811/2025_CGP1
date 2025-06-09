////////////////////////////////////////////////////////////////////////////////
// Filename: textureclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "textureclass.h"
#include "DDSTextureLoader.h"
#include "WICTextureLoader.h"

#pragma comment(lib, "windowscodecs.lib")

using namespace DirectX;

TextureClass::TextureClass()
{
	m_texture = 0;
}


TextureClass::TextureClass(const TextureClass& other)
{
}


TextureClass::~TextureClass()
{
}


bool TextureClass::Initialize(ID3D11Device* device, const WCHAR* filename)
{
	HRESULT result;

	// 파일 확장자 확인
	const wchar_t* ext = wcsrchr(filename, L'.');
	if (ext)
	{
		// DDS 파일인 경우 DDSTextureLoader 사용
		if (_wcsicmp(ext, L".dds") == 0)
		{
			result = CreateDDSTextureFromFile(device, filename, nullptr, &m_texture);
		}
		// 다른 이미지 파일(PNG, JPG 등)인 경우 WICTextureLoader 사용
		else
		{
			result = CreateWICTextureFromFile(device, filename, nullptr, &m_texture);
		}
	}
	else // 확장자가 없는 경우
	{
		return false;
	}

	if (FAILED(result))
	{
		return false;
	}

	return true;
}


bool TextureClass::Initialize(ID3D11Device* device, const void* pData, size_t dataSize)
{
	if (!pData || dataSize == 0)
	{
		return false;
	}

	HRESULT result;

	// 메모리에 있는 이미지 데이터로부터 WIC 텍스처를 생성합니다.
	result = CreateWICTextureFromMemory(device, static_cast<const uint8_t*>(pData), dataSize, nullptr, &m_texture);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureClass::Shutdown()
{
	// Release the texture resource.
	if(m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}