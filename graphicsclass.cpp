////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"
#include <DirectXCollision.h> 

GraphicsClass::GraphicsClass()
{
	m_D3D = nullptr;
	m_TextureShader = nullptr;
	m_LightShader = nullptr;
	m_StaticShader = nullptr;
	m_PBRShader = nullptr;
	m_Bitmap = nullptr;
	m_Text = nullptr;

	m_BoatZOffset = 0.0f;
	m_BoatSpeed = 0.05f;
	m_BoatMovingForward = true;

	m_LighthouseRotationAngle = 0.0f;
	m_LighthouseRotationSpeed = 2.0f;
}


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if (!m_D3D || !m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}


	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader || !m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd)) return false;

	m_LightShader = new LightShaderClass;
	if (!m_LightShader || !m_LightShader->Initialize(m_D3D->GetDevice(), hwnd)) return false;

	m_StaticShader = new StaticShaderClass;
	if (!m_StaticShader || !m_StaticShader->Initialize(m_D3D->GetDevice(), hwnd)) return false;

	m_PBRShader = new PBRShaderClass;
	if (!m_PBRShader || !m_PBRShader->Initialize(m_D3D->GetDevice(), hwnd)) return false;

	m_BillboardShader = new BillboardShaderClass;
	if (!m_BillboardShader || !m_BillboardShader->Initialize(m_D3D->GetDevice(), hwnd))
	{
		MessageBox(hwnd, L"Could not initialize the billboard shader object.", L"Error", MB_OK);
		return false;
	}

	// 단일 텍스처 로드 람다 (기존)
	auto loadModelSingle = [&](const wchar_t* modelFile, const wchar_t* textureFile = nullptr) -> bool {
		auto model = make_unique<ModelClass>();
		if (!model->Initialize(m_D3D->GetDevice(), modelFile, textureFile)) return false;
		m_Models.push_back(move(model));
		return true;
		};

	// 다중 텍스처 로드 람다 (새로 추가)
	auto loadModelMulti = [&](const wchar_t* modelFile, const vector<wstring>& textureFiles) -> bool {
		auto model = make_unique<ModelClass>();
		if (!model->Initialize(m_D3D->GetDevice(), modelFile, textureFiles)) return false;
		m_Models.push_back(move(model));
		return true;
		};


	if (!loadModelSingle(L"./data/floor.obj", L"./data/floor.dds")) return false; // index 0: floor
	m_terrainModelIndices.push_back(0);

	vector<wstring> lighthouseTextures = {
	  L"./data/Lighthouse_Albedo.dds", L"./data/Lighthouse_Normal.dds", L"./data/Lighthouse_Roughness.dds",
	  L"./data/Lighthouse_Emissive.dds", L"./data/Lighthouse_AO.dds"
	};
	if (!loadModelMulti(L"./data/Lighthouse.obj", lighthouseTextures)) return false;
	int lighthouseModelIndex = m_Models.size() - 1; // index 1: lighthouse

	if (!loadModelSingle(L"./data/Bridge.obj", L"./data/Bridge.dds")) return false;       // index 2: bridge
	m_terrainModelIndices.push_back(2);

	if (!loadModelSingle(L"./data/Boat.obj", L"./data/Boat.dds")) return false;           // index 3: boat

	if (!loadModelSingle(L"./data/streetlight.obj", L"./data/streetlight.dds")) return false; // index 4: streetlight

	if (!loadModelSingle(L"./data/Rock.obj", L"./data/Rock.dds")) return false;           // index 5: rock
	m_terrainModelIndices.push_back(5);

	if (!loadModelSingle(L"./data/male.fbx")) return false;                              // index 6: char

	if (!loadModelSingle(L"./data/Mountain.obj", L"./data/Mountain.dds")) return false; // index 7: island
	m_terrainModelIndices.push_back(7);

	if (!loadModelSingle(L"./data/Bullet.fbx")) return false;							// index 8: 총알
	m_bulletModelIndex = m_Models.size() - 1;                        

	if (!loadModelSingle(L"./data/quad.obj", L"./data/airship.dds")) return false; //index 9 빌보드 비행선
	m_billboardModelIndex = m_Models.size() - 1;
	m_airshipPosition = XMFLOAT3(-500.0f, 200.0f, 700.0f);


	//m_Models[6]->LoadAnimation(L"./data/idle.fbx", "idle");
	//m_Models[6]->LoadAnimation(L"./data/running.fbx", "running");
	//m_Models[6]->LoadAnimation(L"./data/attack.fbx", "attack");
	//m_Models[6]->LoadAnimation(L"./data/die.fbx", "die");
	//m_Models[6]->SetAnimationClip("idle");




	// --- 2. 씬에 객체 인스턴스 배치 ---
	m_SceneInstances.clear();

	// 로드된 모델(인덱스)을 사용하여 씬에 여러 개의 인스턴스를 배치합니다.

	// 섬 1 (정적)
	XMFLOAT3 pos_rock1 = { 0.0f, -5.0f, 0.0f };
	m_SceneInstances.push_back({ 5, XMMatrixScaling(2.0f, 0.3f, 1.5f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_rock1)), XMMatrixIdentity(), pos_rock1, 0.f, false, false, false, 0.f, 0.f, false });

	// 섬 2 (정적)
	XMFLOAT3 pos_rock2 = { 100.0f, -5.0f, 100.0f };
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.5f, 0.3f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_rock2)), XMMatrixIdentity(), pos_rock2, 0.f, false, false, false, 0.f, 0.f, false });

	// 섬 3 (정적)
	XMFLOAT3 pos_rock3 = { 0.0f, -4.0f, 150.0f };
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.3f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(0.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_rock3)), XMMatrixIdentity(), pos_rock3, 0.f, false, false, false, 0.f, 0.f, false });

	// 섬 4 (정적)
	XMFLOAT3 pos_rock4 = { -60.0f, -2.0f, 240.0f };
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.4f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_rock4)), XMMatrixIdentity(), pos_rock4, 0.f, false, false, false, 0.f, 0.f, false });

	// 다리 1 (정적)
	XMFLOAT3 pos_bridge1 = { 45.0f, -3.0f, 45.0f };
	m_SceneInstances.push_back({ 2, XMMatrixScaling(5.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(-45.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_bridge1)), XMMatrixIdentity(), pos_bridge1, 0.f, false, false, false, 0.f, 0.f, false });

	// 다리 2 (정적)
	XMFLOAT3 pos_bridge2 = { 27.0f, -1.5f, 140.0f };
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_bridge2)), XMMatrixIdentity(), pos_bridge2, 0.f, false, false, false, 0.f, 0.f, false });

	// 등대 1 (정적)
	XMFLOAT3 pos_lh1 = { -20.0f, -2.0f, 32.0f };
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_lh1)), XMMatrixIdentity(), pos_lh1, 0.f, false, false, false, 0.f, 0.f, false });

	// 등대 2 (정적)
	XMFLOAT3 pos_lh2 = { 120.0f, -4.0f, 130.0f };
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_lh2)), XMMatrixIdentity(), pos_lh2, 0.f, false, false, false, 0.f, 0.f, false });

	// 등대 3 (정적)
	XMFLOAT3 pos_lh3 = { -60.0f, -2.0f, 240.0f };
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_lh3)), XMMatrixIdentity(), pos_lh3, 0.f, false, false, false, 0.f, 0.f, false });

	// 등대 4 (정적)
	XMFLOAT3 pos_lh4 = { 40.0f, -6.0f, 480.0f };
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(4.5f, 4.5f, 4.5f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_lh4)), XMMatrixIdentity(), pos_lh4, 0.f, false, false, false, 0.f, 0.f, false });

	// 등대 5 (정적)
	XMFLOAT3 pos_lh5 = { 10.0f, -4.0f, 600.0f };
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_lh5)), XMMatrixIdentity(), pos_lh5, 0.f, false, false, false, 0.f, 0.f, false });

	// 가로등 1 (정적)
	XMFLOAT3 pos_street1 = { -10.0f, -2.3f, 24.0f };
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_street1)), XMMatrixIdentity(), pos_street1, 0.f, false, false, false, 0.f, 0.f, false });

	// 가로등 2 (정적)
	XMFLOAT3 pos_street2 = { 15.0f, -2.3f, 27.0f };
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_street2)), XMMatrixIdentity(), pos_street2, 0.f, false, false, false, 0.f, 0.f, false });

	// 대형 섬 (정적)
	XMFLOAT3 pos_mountain = { 100.0f, -6.0f, -150.0f };
	m_SceneInstances.push_back({ 7, XMMatrixScaling(200.0f, 150.0f, 200.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslationFromVector(XMLoadFloat3(&pos_mountain)), XMMatrixIdentity(), pos_mountain, 0.f, false, false, false, 0.f, 0.f, false });


	// --- 캐릭터와 보트의 자연스러운 배치 ---

	// [캐릭터]
	XMFLOAT3 charInitialPos = { -20.0f, -1.8f, 150.0f };
	float charInitialRotation = XMConvertToRadians(180.0f);
	m_SceneInstances.push_back({
		6,                                                                          // modelIndex
		XMMatrixIdentity(),                                                         // worldTransform
		XMMatrixScaling(0.02f, 0.02f, 0.02f) * XMMatrixRotationY(charInitialRotation),// baseTransform
		charInitialPos,                                                             // currentPosition
		charInitialRotation,                                                        // currentYRotation
		true,                                                                       // canMove
		false,                                                                      // isAnimated
		false,                                                                      // movingForward
		0.0f,                                                                       // animationOffset
		3.0f,                                                                       // collisionRadius
		false                                                                       // isMarkedForRemoval 
		});


	// [움직이는 보트]
	XMFLOAT3 boatInitialPos = { -10.0f, -5.0f, 75.0f };
	float boatInitialRotation = XMConvertToRadians(0.0f);
	m_SceneInstances.push_back({
		3,                                                                          // modelIndex
		XMMatrixIdentity(),                                                         // worldTransform
		XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(boatInitialRotation), // baseTransform
		boatInitialPos,                                                             // currentPosition
		boatInitialRotation,                                                        // currentYRotation
		false,                                                                      // canMove
		true,                                                                       // isAnimated
		true,                                                                       // movingForward
		0.0f,                                                                       // animationOffset
		0.0f,                                                                       // collisionRadius
		false                                                                       // isMarkedForRemoval
		});

	// [정박한 보트]
	XMFLOAT3 parkedBoatPos = { 40.0f, -5.0f, 80.0f };
	float parkedBoatRotation = XMConvertToRadians(100.0f);
	m_SceneInstances.push_back({
		3,                                                                          // modelIndex
		XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(parkedBoatRotation) * XMMatrixTranslation(parkedBoatPos.x, parkedBoatPos.y, parkedBoatPos.z), // worldTransform
		XMMatrixIdentity(),                                                         // baseTransform
		parkedBoatPos,                                                              // currentPosition
		parkedBoatRotation,                                                         // currentYRotation
		false,                                                                      // canMove
		false,                                                                      // isAnimated
		false,                                                                      // movingForward
		0.0f,                                                                       // animationOffset
		0.0f,                                                                       // collisionRadius
		false                                                                       // isMarkedForRemoval
		});



	// [배경용 floor]
	m_SceneInstances.push_back({ 0, XMMatrixScaling(5.0f, 1.0f, 5.0f) * XMMatrixTranslation(0.0f, -6.2f, 0.0f), XMMatrixIdentity(), {}, 0.f, false, false, false, 0.f, 0.f, false });


	// --- 광원 객체 초기화 ---
	m_Lights.clear(); // 기존 광원 삭제
	m_Lights.resize(4);

	// 1. 전역 방향성 조명 (달빛처럼 약간 위에서 비스듬히)
	m_Lights[0] = new LightClass();
	m_Lights[0]->SetLightType(LightType::Directional);
	m_Lights[0]->SetDirection(-0.5f, -0.7f, -0.5f); // 비스듬한 방향
	m_Lights[0]->SetDiffuseColor(0.4f, 0.4f, 0.5f, 1.0f); // 약한 푸른빛
	m_Lights[0]->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Lights[0]->SetSpecularPower(64.0f);

	// 2. 등대 1의 스포트라이트
	m_Lights[1] = new LightClass();
	m_Lights[1]->SetLightType(LightType::Spot);
	m_Lights[1]->SetPosition(40.0f, 64.0f, 480.0f); // 등대 모델 위쪽
	m_Lights[1]->SetDirection(0.0f, -0.3f, 1.0f); // 약간 아래 앞쪽을 향함
	m_Lights[1]->SetDiffuseColor(50.0f, 50.0f, 20.0f, 1.0f); // 강한 노란빛
	m_Lights[1]->SetSpecularColor(50.0f, 50.0f, 50.0f, 1.0f);
	m_Lights[1]->SetSpecularPower(256.0f);
	m_Lights[1]->SetSpotlightAngle(15.0f, 25.0f);

	// 3. 가로등 1의 스포트라이트
	m_Lights[2] = new LightClass();
	m_Lights[2]->SetLightType(LightType::Spot);
	m_Lights[2]->SetPosition(-10.0f, 5.0f, 24.0f);
	m_Lights[2]->SetDirection(0.0f, -1.0f, 0.0f); // 바로 아래를 비춤
	m_Lights[2]->SetDiffuseColor(80.0f, 60.0f, 20.0f, 1.0f);
	m_Lights[2]->SetSpecularColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[2]->SetSpecularPower(128.0f);
	m_Lights[2]->SetSpotlightAngle(40.0f, 60.0f);

	// 4. 가로등 2의 스포트라이트
	m_Lights[3] = new LightClass();
	m_Lights[3]->SetLightType(LightType::Spot);
	m_Lights[3]->SetPosition(15.0f, 5.0f, 27.0f);
	m_Lights[3]->SetDirection(0.0f, -1.0f, 0.0f);
	m_Lights[3]->SetDiffuseColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[3]->SetSpecularColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[3]->SetSpecularPower(128.0f);
	m_Lights[3]->SetSpotlightAngle(40.0f, 60.0f);


	// --- UI 객체 초기화 ---
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap || !m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/space.dds", screenWidth, screenHeight))
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	XMMATRIX baseViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	m_Text = new TextClass;
	if (!m_Text || !m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix)) return false;

	return true;
}


void GraphicsClass::Shutdown()
{
	for (auto& light : m_Lights) { delete light; light = nullptr; }
	m_Lights.clear();
	if (m_Text) { m_Text->Shutdown(); delete m_Text; m_Text = nullptr; }
	if (m_Bitmap) { m_Bitmap->Shutdown(); delete m_Bitmap; m_Bitmap = nullptr; }
	if (m_StaticShader) { m_StaticShader->Shutdown(); delete m_StaticShader; m_StaticShader = nullptr; }
	if (m_LightShader) { m_LightShader->Shutdown(); delete m_LightShader; m_LightShader = nullptr; }
	if (m_TextureShader) { m_TextureShader->Shutdown(); delete m_TextureShader; m_TextureShader = nullptr; }
	m_Models.clear(); // unique_ptr가 모든 ModelClass 메모리 자동 해제
	m_SceneInstances.clear();
	if (m_D3D) { m_D3D->Shutdown(); delete m_D3D; m_D3D = nullptr; }
	if (m_PBRShader) { m_PBRShader->Shutdown(); delete m_PBRShader; m_PBRShader = nullptr; }
	if (m_StaticShader) { m_StaticShader->Shutdown(); delete m_StaticShader; m_StaticShader = nullptr; }
	if (m_BillboardShader)
	{
		m_BillboardShader->Shutdown();
		delete m_BillboardShader;
		m_BillboardShader = nullptr;
	}
}



bool GraphicsClass::Frame(int fps, int cpu, CameraClass* gameCamera, InputClass* input, float deltaTime)
{
	// =================================================================
   // 1. 입력 처리 (Input Processing)
   // =================================================================
	if (input->IsMouseButtonPressed(0))
	{
		BulletInstance newBullet;
		newBullet.position = gameCamera->GetPosition();
		XMFLOAT3 forward, right, up;
		gameCamera->GetDirectionVectors(forward, right, up);
		newBullet.direction = forward;
		newBullet.speed = 50.0f; // 속도 현실적으로 조절
		newBullet.lifeTime = 3.0f;
		newBullet.collisionRadius = 0.2f;
		newBullet.isMarkedForRemoval = false;
		m_Bullets.push_back(newBullet);
	}

	// =================================================================
	// 2. 객체 상태 업데이트 (Update State)
	// =================================================================

	// 2-1. 모든 총알 업데이트
	for (auto& bullet : m_Bullets)
	{
		XMVECTOR pos = XMLoadFloat3(&bullet.position);
		XMVECTOR dir = XMLoadFloat3(&bullet.direction);
		pos += dir * bullet.speed * deltaTime;
		XMStoreFloat3(&bullet.position, pos);
		bullet.lifeTime -= deltaTime;
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		if (XMVector3Equal(XMVector3Normalize(dir), XMVector3Normalize(up)) || XMVector3Equal(XMVector3Normalize(dir), -XMVector3Normalize(up)))
		{
			up = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
		}
		XMMATRIX rotationMatrix = XMMatrixLookToLH(XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f), dir, up);
		rotationMatrix = XMMatrixInverse(nullptr, rotationMatrix);
		bullet.worldTransform = XMMatrixScaling(0.1f, 0.1f, 0.1f) * rotationMatrix * XMMatrixTranslationFromVector(pos);
	}

	// 2-2. 모든 씬 객체(AI, 보트 등) 업데이트
	XMFLOAT3 cameraPosition = gameCamera->GetPosition();
	for (auto& instance : m_SceneInstances)
	{
		if (instance.isAnimated) // 움직이는 보트
		{
			float moveRange = 20.0f;
			float boatSpeed = 5.0f;
			if (instance.movingForward) {
				instance.animationOffset += boatSpeed * deltaTime;
				if (instance.animationOffset > moveRange) instance.movingForward = false;
			}
			else {
				instance.animationOffset -= boatSpeed * deltaTime;
				if (instance.animationOffset < -moveRange) instance.movingForward = true;
			}
			XMMATRIX animationTransform = XMMatrixTranslation(0.0f, 0.0f, instance.animationOffset);
			instance.worldTransform = instance.baseTransform * animationTransform * XMMatrixTranslation(instance.currentPosition.x, instance.currentPosition.y, instance.currentPosition.z);
		}
		else if (instance.canMove) // AI 캐릭터
		{
			//ModelClass* model = m_Models[instance.modelIndex].get();
			float dx = cameraPosition.x - instance.currentPosition.x;
			float dz = cameraPosition.z - instance.currentPosition.z;
			float distance = sqrt(dx * dx + dz * dz);
			float followDistance = 30.0f;
			float stopDistance = 3.0f;
			float moveSpeed = 5.0f;

			if (distance < followDistance)
			{
				instance.currentYRotation = atan2(dx, dz);
				if (distance > stopDistance) {
					//model->SetAnimationClip("running"); // 애니메이션 비활성화
					XMVECTOR moveDirection = XMVector3Normalize(XMVectorSet(dx, 0.0f, dz, 0.0f));
					XMVECTOR movement = moveDirection * moveSpeed * deltaTime;
					instance.currentPosition.x += XMVectorGetX(movement);
					instance.currentPosition.z += XMVectorGetZ(movement);

					// 지형 높이 적용
					float groundHeight;
					if (FindGroundHeight(instance.currentPosition.x, instance.currentPosition.z, groundHeight))
					{
						instance.currentPosition.y = groundHeight;
					}
				}
				else {
					//model->SetAnimationClip("idle"); // 애니메이션 비활성화
				}
			}
			else {
				//model->SetAnimationClip("idle"); // 애니메이션 비활성화
			}
			instance.worldTransform = instance.baseTransform * XMMatrixRotationY(instance.currentYRotation) * XMMatrixTranslation(instance.currentPosition.x, instance.currentPosition.y, instance.currentPosition.z);
		}
	}

	// 2-3. 기타 월드 업데이트
	for (auto& model : m_Models)
	{
		model->UpdateAnimation(deltaTime); // 애니메이션이 없어도 호출은 안전함
	}
	m_LighthouseRotationAngle += m_LighthouseRotationSpeed * deltaTime;
	if (m_LighthouseRotationAngle > XM_2PI) { m_LighthouseRotationAngle -= XM_2PI; }
	float newDirX = sinf(m_LighthouseRotationAngle);
	float newDirZ = cosf(m_LighthouseRotationAngle);
	if (m_Lights.size() > 1 && m_Lights[1] != nullptr) { m_Lights[1]->SetDirection(newDirX, -0.3f, newDirZ); }
	m_airshipPosition.x += deltaTime * 3.0f;
	if (m_airshipPosition.x > 300.0f) { m_airshipPosition.x = -300.0f; }

	// =================================================================
	// 3. 충돌 감지 (Collision Detection)
	// =================================================================
	for (auto& bullet : m_Bullets)
	{
		if (bullet.isMarkedForRemoval) continue;
		for (auto& instance : m_SceneInstances)
		{
			if (instance.collisionRadius <= 0.0f) continue;
			XMVECTOR bulletPos = XMLoadFloat3(&bullet.position);
			XMVECTOR instancePos = XMLoadFloat3(&instance.currentPosition);
			XMVECTOR distanceSquared = XMVector3LengthSq(bulletPos - instancePos);
			float combinedRadius = bullet.collisionRadius + instance.collisionRadius;
			if (XMVectorGetX(distanceSquared) <= (combinedRadius * combinedRadius))
			{
				bullet.isMarkedForRemoval = true;
				instance.isMarkedForRemoval = true;
				break;
			}
		}
	}

	// =================================================================
	// 4. 객체 제거 (Cleanup)
	// =================================================================
	m_Bullets.erase(std::remove_if(m_Bullets.begin(), m_Bullets.end(), [](const BulletInstance& b) { return b.lifeTime <= 0.0f || b.isMarkedForRemoval; }), m_Bullets.end());
	m_SceneInstances.erase(std::remove_if(m_SceneInstances.begin(), m_SceneInstances.end(), [](const SceneObjectInstance& inst) { return inst.isMarkedForRemoval; }), m_SceneInstances.end());

	// =================================================================
	// 5. UI 업데이트 및 렌더링 (UI & Render)
	// =================================================================
	if (!m_Text->SetFPS(fps, m_D3D->GetDeviceContext())) return false;
	if (!m_Text->SetCPU(cpu, m_D3D->GetDeviceContext())) return false;

	return Render(gameCamera);
}

bool GraphicsClass::Render(CameraClass* gameCamera)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix, uiViewMatrix;

	// 1. 프레임 시작
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	gameCamera->GetViewMatrix(viewMatrix);      // 3D 씬을 위한 뷰 행렬
	m_D3D->GetWorldMatrix(worldMatrix);         // 기본 월드 행렬
	m_D3D->GetProjectionMatrix(projectionMatrix); // 3D 씬을 위한 투영 행렬
	m_D3D->GetOrthoMatrix(orthoMatrix);         // 2D 렌더링을 위한 직교 행렬

	uiViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	// 2. 배경 비트맵 렌더링 (Z-버퍼 끄기)
	// 배경은 깊이와 상관없이 가장 먼저 그려져야 합니다.
	m_D3D->TurnZBufferOff();
	{
		// 비트맵의 버텍스/인덱스 버퍼를 활성화하고 위치를 설정합니다.
		m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
		// 텍스처 셰이더를 사용해 비트맵을 실제로 그립니다.
		m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, uiViewMatrix, orthoMatrix, m_Bitmap->GetTexture());
	}
	m_D3D->TurnZBufferOn(); // 3D 씬을 그리기 위해 Z-버퍼를 다시 켭니다.

	// 3. 모델 렌더링
	for (const auto& instance : m_SceneInstances)
	{
		ModelClass* model = m_Models[instance.modelIndex].get();
		model->Render(m_D3D->GetDeviceContext());
		XMMATRIX finalWorldMatrix = instance.worldTransform;

		switch (model->GetShaderType())
		{
		case ModelClass::ShaderType::PBR:
			m_PBRShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTextures(), // 다중 텍스처 전달
				m_Lights, gameCamera);
			break;

		case ModelClass::ShaderType::Animated:
			m_LightShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), // 단일 텍스처 전달
				m_Lights, model->GetFinalBoneTransforms(), gameCamera);
			break;

		case ModelClass::ShaderType::Default:
		default:
			m_StaticShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), // 단일 텍스처 전달
				m_Lights, gameCamera);
			break;
		}
	}

	

	for (const auto& bullet : m_Bullets)
	{

		//총알
		ModelClass* bulletModel = m_Models[m_bulletModelIndex].get();
		bulletModel->Render(m_D3D->GetDeviceContext());

		// 총알은 애니메이션이 없으므로 StaticShader 사용
		m_StaticShader->Render(m_D3D->GetDeviceContext(), bulletModel->GetIndexCount(),
			bullet.worldTransform, viewMatrix, projectionMatrix,
			bulletModel->GetTexture(),
			m_Lights, gameCamera);
	}

	//비행선 빌보드
	m_D3D->TurnOnAlphaBlending();
	ModelClass* billboardModel = m_Models[m_billboardModelIndex].get();
	billboardModel->Render(m_D3D->GetDeviceContext());

	// 비행선의 월드 행렬 (크기와 위치만 설정, 회전은 셰이더가 담당)
	XMMATRIX billboardWorldMatrix = XMMatrixScaling(200.0f, 100.0f, 1.0f) * XMMatrixTranslationFromVector(XMLoadFloat3(&m_airshipPosition));

	// 카메라의 위치와 상향 벡터 가져오기
	XMFLOAT3 camPos = gameCamera->GetPosition();
	XMFLOAT3 camUp;
	XMFLOAT3 camForward, camRight;
	gameCamera->GetDirectionVectors(camForward, camRight, camUp);

	m_BillboardShader->Render(m_D3D->GetDeviceContext(), billboardModel->GetIndexCount(),
		billboardWorldMatrix, viewMatrix, projectionMatrix,
		billboardModel->GetTexture(),
		camPos, camUp);

	// 사용했던 렌더링 상태를 원래대로 복구
	m_D3D->TurnOffAlphaBlending();

	// 4. 2D UI 렌더링 (Z-버퍼 끄기)
	m_D3D->TurnZBufferOff();
	{
		// 텍스트 렌더링
		m_D3D->TurnOnAlphaBlending();
		m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
		m_D3D->TurnOffAlphaBlending();
	}
	m_D3D->TurnZBufferOn();

	// 5. 프레임 종료
	m_D3D->EndScene();

	return true;
}

bool GraphicsClass::Raycast(const XMFLOAT3& rayOrigin, const XMFLOAT3& rayDirection, float& out_distance) const
{
	// XMVECTOR 타입으로 변환
	XMVECTOR origin = XMLoadFloat3(&rayOrigin);
	XMVECTOR direction = XMLoadFloat3(&rayDirection);

	float closest_dist = FLT_MAX; // 가장 가까운 충돌 거리를 저장할 변수 (최대값으로 초기화)
	bool hit = false;

	// 지형으로 지정된 모든 모델에 대해 검사
	for (int modelIndex : m_terrainModelIndices)
	{
		const auto& model = m_Models[modelIndex];
		const auto& vertices = model->GetVertices();
		const auto& indices = model->GetIndices();

		// 씬에 배치된 해당 모델의 모든 인스턴스를 찾아서 검사
		for (const auto& instance : m_SceneInstances)
		{
			if (instance.modelIndex != modelIndex) continue;

			// 모델의 월드 변환 행렬과 그 역행렬을 가져옴
			XMMATRIX worldMatrix = instance.worldTransform;
			XMMATRIX invWorldMatrix = XMMatrixInverse(nullptr, worldMatrix);

			// 광선을 모델의 로컬 공간으로 변환
			XMVECTOR localOrigin = XMVector3TransformCoord(origin, invWorldMatrix);
			XMVECTOR localDirection = XMVector3TransformNormal(direction, invWorldMatrix);

			// 모델의 모든 삼각형과 교차 테스트
			for (size_t i = 0; i < indices.size(); i += 3)
			{
				// 삼각형의 세 꼭짓점 인덱스
				unsigned long i0 = indices[i];
				unsigned long i1 = indices[i + 1];
				unsigned long i2 = indices[i + 2];

				// 세 꼭짓점의 로컬 좌표
				XMVECTOR v0 = XMLoadFloat3(&vertices[i0].Position);
				XMVECTOR v1 = XMLoadFloat3(&vertices[i1].Position);
				XMVECTOR v2 = XMLoadFloat3(&vertices[i2].Position);

				float dist; // 충돌 거리를 저장할 변수
				if (DirectX::TriangleTests::Intersects(localOrigin, localDirection, v0, v1, v2, dist))
				{
					if (dist < closest_dist)
					{
						closest_dist = dist;
						hit = true;
					}
				}
			}
		}
	}

	if (hit)
	{
		out_distance = closest_dist;
	}

	return hit;
}

// 특정 (x, z) 좌표의 바닥 높이를 찾는 함수
bool GraphicsClass::FindGroundHeight(float x, float z, float& out_height) const
{
	// 캐릭터 위치 바로 위 공중에서 아래로 광선을 쏜다.
	XMFLOAT3 rayOrigin = { x, 1000.0f, z };   // 시작점 (충분히 높은 곳)
	XMFLOAT3 rayDirection = { 0.0f, -1.0f, 0.0f }; // 방향 (아래)

	float distance;
	if (Raycast(rayOrigin, rayDirection, distance))
	{
		// 충돌했다면, 높이 = 시작점 높이 - 충돌 거리
		out_height = rayOrigin.y - distance;
		return true;
	}

	// 충돌한 지형이 없다면 실패
	return false;
}