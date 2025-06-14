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

	// ���� ���� ������
	XMFLOAT3 currentPosition;
	float currentYRotation;
	bool canMove = false;
	bool isAnimated = false;
	

	// isAnimated�� ���� ����
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
	float lifeTime; // �Ѿ��� ������� �ð� (��)
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

	// ������ �𵨵��� �����ϴ� ���� (�޸� ȿ��ȭ)
	std::vector<std::unique_ptr<ModelClass>> m_Models;
	// ���� ��ġ�� ��ü �ν��Ͻ����� ���
	std::vector<SceneObjectInstance> m_SceneInstances;
	std::vector<BulletInstance> m_Bullets; // <<-- �Ѿ� ��� �߰�
	int m_bulletModelIndex = -1;

	// ���̴� ��ü��
	TextureShaderClass* m_TextureShader;
	LightShaderClass* m_LightShader;     // �ִϸ��̼� �𵨿�
	StaticShaderClass* m_StaticShader;   // ���� �𵨿�
	PBRShaderClass* m_PBRShader;

	// UI ��ü��
	BitmapClass* m_Bitmap;
	TextClass* m_Text;

	// ���� ��ü���� ���ͷ� �����մϴ�.
	std::vector<LightClass*> m_Lights;

	float m_LighthouseRotationAngle; // ���� ȸ�� ���� (����)
	float m_LighthouseRotationSpeed; // ȸ�� �ӵ� (����/��)

	std::vector<int> m_terrainModelIndices; // ���� ���� �ε����� ������ ����

	BillboardShaderClass* m_BillboardShader; // <<-- ���̴� ��ü �߰�

	int m_billboardModelIndex = -1; // <<-- ������� �簢�� �� �ε���
	XMFLOAT3 m_airshipPosition;     // <<-- ���༱ ��ġ

};

#endif