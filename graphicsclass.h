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
	int modelIndex;               // m_Models 벡터에 있는 모델의 인덱스
	XMMATRIX worldTransform;      // 객체의 월드 변환 행렬
	bool isAnimated = false; //움직이는지
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

	// 고유한 모델들을 저장하는 벡터 (메모리 효율화)
	std::vector<std::unique_ptr<ModelClass>> m_Models;
	// 씬에 배치될 객체 인스턴스들의 목록
	std::vector<SceneObjectInstance> m_SceneInstances;

	// 셰이더 객체들
	TextureShaderClass* m_TextureShader;
	LightShaderClass* m_LightShader;

	// UI 객체들
	BitmapClass* m_Bitmap;
	TextClass* m_Text;

	// 광원 객체들을 벡터로 관리합니다.
	std::vector<LightClass*> m_Lights;
};

#endif