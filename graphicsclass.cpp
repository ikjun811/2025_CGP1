////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


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


	if (!loadModelSingle(L"./data/floor.obj", L"./data/floor.dds")) return false; // index 0        // index 0: floor

	vector<wstring> lighthouseTextures = {
	  L"./data/Lighthouse_Albedo.dds", // t0: Diffuse
	  L"./data/Lighthouse_Normal.dds", // t1: Normal
	  L"./data/Lighthouse_Roughness.dds", // t2: Specular/Roughness
	  L"./data/Lighthouse_Emissive.dds", // t3: Emissive
	  L"./data/Lighthouse_AO.dds" // t4: Ambient Occlusion
	};
	if (!loadModelMulti(L"./data/Lighthouse.obj", lighthouseTextures)) return false;
	int lighthouseModelIndex = m_Models.size() - 1; // 등대 모델 인덱스 저장

	if (!loadModelSingle(L"./data/Bridge.obj", L"./data/Bridge.dds")) return false;       // index 2: bridge
	if (!loadModelSingle(L"./data/Boat.obj", L"./data/Boat.dds")) return false;           // index 3: boat
	if (!loadModelSingle(L"./data/streetlight.obj", L"./data/streetlight.dds")) return false; // index 4: streetlight
	if (!loadModelSingle(L"./data/Rock.obj", L"./data/Rock.dds")) return false;           // index 5: rock
	if (!loadModelSingle(L"./data/male.fbx")) return false; // index 6 char
	//if (!loadModel(L"./data/character.fbx")) return false;

	/*	m_Models[6]->LoadAnimation(L"./data/male_idle1_200f.fbx", "idle");
	m_Models[6]->LoadAnimation(L"./data/male_running_20f.fbx", "running");
	m_Models[6]->SetAnimationClip("idle"); */

	//m_Models[6]->LoadAnimation(L"./data/idle.fbx", "idle");
	//m_Models[6]->LoadAnimation(L"./data/running.fbx", "running");
	//m_Models[6]->LoadAnimation(L"./data/attack.fbx", "attack");
	//m_Models[6]->LoadAnimation(L"./data/die.fbx", "die");
	//m_Models[6]->SetAnimationClip("idle");




	// --- 2. 씬에 객체 인스턴스 배치 ---
	m_SceneInstances.clear();

	// 로드된 모델(인덱스)을 사용하여 씬에 여러 개의 인스턴스를 배치합니다.

	// 섬 1]
	m_SceneInstances.push_back({ 5, XMMatrixScaling(2.0f, 0.3f, 1.5f) * XMMatrixTranslation(0.0f, -5.0f, 0.0f) }); // index 5: rock

	// 섬 2
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.5f, 0.3f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslation(100.0f, -5.0f, 100.0f) });

	// 섬 3
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.3f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(0.0f)) * XMMatrixTranslation(0.0f, -4.0f, 150.0f) });

	// 섬 4
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.4f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(-60.0f, -2.0f, 240.0f) });


	// 다리

	// 다리 1
	m_SceneInstances.push_back({ 2, XMMatrixScaling(5.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(-45.0f)) * XMMatrixTranslation(45.0f, -3.0f, 45.0f) }); // index 2: bridge

	// 다리 2
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslation(27.0f, -1.5f, 140.0f) });


	// 등대 가로등

	// 등대 1
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(-20.0f, -1.5f, 32.0f) }); // index 1: lighthouse

	// 등대 2
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(120.0f, -4.0f, 130.0f) });

	//등대 3
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(-60.0f, -2.0f, 240.0f) });

	//등대 4
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(4.5f, 4.5f, 4.5f) * XMMatrixTranslation(40.0f, -6.0f, 480.0f) });

	//등대 5
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(10.0f, -4.0f, 600.0f) });

	// 가로등
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(-10.0f, -2.3f, 24.0f) }); // index 4: streetlight
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(15.0f, -2.3f, 27.0f) });


	// --- 캐릭터와 보트의 자연스러운 배치 ---

	// [캐릭터] 왼쪽 앞 섬(섬 2)의 다리 입구에 서서 중앙 섬을 바라보는 구도.
	XMVECTOR charInitialPos = { -28.0f, -1.5f, 18.0f };
	m_SceneInstances.push_back({
		6,
		XMMatrixScaling(0.02f, 0.02f, 0.02f) * XMMatrixRotationY(XMConvertToRadians(45.0f)) * XMMatrixTranslationFromVector(charInitialPos),
		false,
		charInitialPos
		}); // index 6: char (스케일은 작게 유지)

	// [움직이는 보트] 섬들 사이를 항해하는 모습.
	XMVECTOR boatInitialPos = { -10.0f, -5.0f, 75.0f };
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslationFromVector(boatInitialPos), true, boatInitialPos }); // index 3: boat

	// [정박한 보트] 중앙 섬(섬 1) 근처에 정박한 모습.
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(XMConvertToRadians(100.0f)) * XMMatrixTranslation(40.0f, -5.0f, 80.0f) });

	// [배경용 floor] 바다 역할을 할 기본 바닥. 매우 넓게 깔아줍니다.
	m_SceneInstances.push_back({ 0, XMMatrixScaling(5.0f, 1.0f, 5.0f) * XMMatrixTranslation(0.0f, -6.0f, 0.0f) }); // index 0: floor


	// --- 광원 객체 초기화 ---
	m_Lights.clear(); // 기존 광원 삭제
	m_Lights.resize(4);

	// 1. 전역 방향성 조명 (달빛처럼 약간 위에서 비스듬히)
	m_Lights[0] = new LightClass();
	m_Lights[0]->SetLightType(LightType::Directional);
	m_Lights[0]->SetDirection(0.5f, -0.7f, 0.5f); // 비스듬한 방향
	m_Lights[0]->SetDiffuseColor(0.8f, 0.8f, 1.0f, 1.0f); // 약한 푸른빛
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
}



bool GraphicsClass::Frame(int fps, int cpu, CameraClass* gameCamera, float deltaTime)
{
	// 보트 직선 운동 업데이트
	float moveRange = 5.0f;
	if (m_BoatMovingForward)
	{
		m_BoatZOffset += m_BoatSpeed;
		if (m_BoatZOffset > moveRange) m_BoatMovingForward = false;
	}
	else
	{
		m_BoatZOffset -= m_BoatSpeed;
		if (m_BoatZOffset < -moveRange) m_BoatMovingForward = true;
	}

	// 애니메이션이 적용될 객체의 월드 행렬을 업데이트합니다.
	for (auto& instance : m_SceneInstances)
	{
		if (instance.isAnimated)
		{

			XMMATRIX scaleMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);

			XMMATRIX translationMatrix = XMMatrixTranslation(
				XMVectorGetX(instance.initialPosition),
				XMVectorGetY(instance.initialPosition),
				XMVectorGetZ(instance.initialPosition) + m_BoatZOffset 
			);
			// 3. 최종 월드 행렬을 조합합니다.
			instance.worldTransform = scaleMatrix * translationMatrix;
		}
	}

	//float deltaTime = 1.0f / 60.0f; 
	for (auto& model : m_Models)
	{
		model->UpdateAnimation(deltaTime);
	}

	//등대 조명 회전
	m_LighthouseRotationAngle += m_LighthouseRotationSpeed * deltaTime;
	if (m_LighthouseRotationAngle > XM_2PI)
	{
		m_LighthouseRotationAngle -= XM_2PI;
	}

	float newDirX = sinf(m_LighthouseRotationAngle);
	float newDirZ = cosf(m_LighthouseRotationAngle);

	if (m_Lights.size() > 1 && m_Lights[1] != nullptr)
	{
		// SetDirection을 호출하여 조명의 방향을 실시간으로 변경
		m_Lights[1]->SetDirection(newDirX, -0.3f, newDirZ);
	}

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


