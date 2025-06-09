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

	// ���� Ȯ���� Ȯ��
	const wchar_t* ext = wcsrchr(filename, L'.');
	if (ext)
	{
		// DDS ������ ��� DDSTextureLoader ���
		if (_wcsicmp(ext, L".dds") == 0)
		{
			result = CreateDDSTextureFromFile(device, filename, nullptr, &m_texture);
		}
		// �ٸ� �̹��� ����(PNG, JPG ��)�� ��� WICTextureLoader ���
		else
		{
			result = CreateWICTextureFromFile(device, filename, nullptr, &m_texture);
		}
	}
	else // Ȯ���ڰ� ���� ���
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

	// �޸𸮿� �ִ� �̹��� �����ͷκ��� WIC �ؽ�ó�� �����մϴ�.
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