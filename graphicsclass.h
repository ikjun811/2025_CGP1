////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _GRAPHICSCLASS_H_
#define _GRAPHICSCLASS_H_


///////////////////////
// MY CLASS INCLUDES //
///////////////////////
#include "d3dclass.h"
#include "cameraclass.h"
#include "modelclass.h"
#include "lightshaderclass.h"
#include "lightclass.h"
#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"

#include <vector>
#include <string>
#include <memory>

/////////////
// GLOBALS //
/////////////
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;


struct SceneObjectInstance
{
	int modelIndex;               // m_Models ���Ϳ� �ִ� ���� �ε���
	XMMATRIX worldTransform;      // ��ü�� ���� ��ȯ ���
	bool isAnimated = false; //�����̴���
};


////////////////////////////////////////////////////////////////////////////////
// Class name: GraphicsClass
////////////////////////////////////////////////////////////////////////////////
class GraphicsClass
{
public:
	GraphicsClass();
	GraphicsClass(const GraphicsClass&);
	~GraphicsClass();

	bool Initialize(int screenWidth, int screenHeight, HWND hwnd);
	void Shutdown();
	bool Frame(int fps, int cpu, CameraClass* gameCamera);


private:
	bool Render(CameraClass* gameCamera);

	float m_BoatZOffset;
	float m_BoatSpeed;
	bool m_BoatMovingForward;

	float m_ObjectSpeed;
	float m_ObjectZOffset;
	bool m_ObjectMovingForward;
	float m_ObjectStartPosition;

private:
	D3DClass* m_D3D;

	// ������ �𵨵��� �����ϴ� ���� (�޸� ȿ��ȭ)
	std::vector<std::unique_ptr<ModelClass>> m_Models;
	// ���� ��ġ�� ��ü �ν��Ͻ����� ���
	std::vector<SceneObjectInstance> m_SceneInstances;

	// ���̴� ��ü��
	TextureShaderClass* m_TextureShader;
	LightShaderClass* m_LightShader;

	// UI ��ü��
	BitmapClass* m_Bitmap;
	TextClass* m_Text;

	// ���� ��ü���� ���ͷ� �����մϴ�.
	std::vector<LightClass*> m_Lights;
};

#endif