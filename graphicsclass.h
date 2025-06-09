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
#include "staticshaderclass.h"
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
	int modelIndex;
	XMMATRIX worldTransform;
	bool isAnimated = false;

	// 애니메이션을 위한 추가 데이터 (선택적이지만 좋은 방법)
	XMVECTOR initialPosition; // 객체의 초기 위치
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
	bool Frame(int fps, int cpu, CameraClass* gameCamera, float deltaTime);


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
	LightShaderClass* m_LightShader;     // 애니메이션 모델용
	StaticShaderClass* m_StaticShader;   // 정적 모델용

	// UI 객체들
	BitmapClass* m_Bitmap;
	TextClass* m_Text;

	// 광원 객체들을 벡터로 관리합니다.
	std::vector<LightClass*> m_Lights;

	float m_LighthouseRotationAngle; // 현재 회전 각도 (라디안)
	float m_LighthouseRotationSpeed; // 회전 속도 (라디안/초)


};

#endif