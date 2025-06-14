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
#include "pbrshaderclass.h"
#include "lightclass.h"
#include "textureshaderclass.h"
#include "bitmapclass.h"
#include "textclass.h"
#include <directxmath.h>
#include "billboardshaderclass.h"

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
	XMMATRIX baseTransform;

	// 동적 상태 변수들
	XMFLOAT3 currentPosition;
	float currentYRotation;
	bool canMove = false;
	bool isAnimated = false;
	

	// isAnimated를 위한 상태
	bool movingForward = true;
	float animationOffset = 0.0f;

	float collisionRadius = 0.0f;
	bool isMarkedForRemoval = false;
};

struct BulletInstance
{
	XMMATRIX worldTransform;
	XMFLOAT3 position;
	XMFLOAT3 direction;
	float speed;
	float lifeTime; // 총알이 살아있을 시간 (초)
	float collisionRadius = 0.0f;
	bool isMarkedForRemoval = false;
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
	bool Frame(int fps, int cpu, CameraClass* gameCamera, InputClass* input, float deltaTime);


private:
	bool Render(CameraClass* gameCamera);

	bool Raycast(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float& out_distance) const;
	bool FindGroundHeight(float x, float z, float& out_height) const;

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
	std::vector<BulletInstance> m_Bullets; // <<-- 총알 목록 추가
	int m_bulletModelIndex = -1;

	// 셰이더 객체들
	TextureShaderClass* m_TextureShader;
	LightShaderClass* m_LightShader;     // 애니메이션 모델용
	StaticShaderClass* m_StaticShader;   // 정적 모델용
	PBRShaderClass* m_PBRShader;

	// UI 객체들
	BitmapClass* m_Bitmap;
	TextClass* m_Text;

	// 광원 객체들을 벡터로 관리합니다.
	std::vector<LightClass*> m_Lights;

	float m_LighthouseRotationAngle; // 현재 회전 각도 (라디안)
	float m_LighthouseRotationSpeed; // 회전 속도 (라디안/초)

	std::vector<int> m_terrainModelIndices; // 지형 모델의 인덱스를 저장할 벡터

	BillboardShaderClass* m_BillboardShader; // <<-- 셰이더 객체 추가

	int m_billboardModelIndex = -1; // <<-- 빌보드용 사각형 모델 인덱스
	XMFLOAT3 m_airshipPosition;     // <<-- 비행선 위치

};

#endif