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
	m_Bitmap = nullptr;
	m_Text = nullptr;

	m_BoatZOffset = 0.0f;
	m_BoatSpeed = 0.05f;
	m_BoatMovingForward = true;
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

	auto loadModel = [&](const wchar_t* modelFile, const wchar_t* textureFile = nullptr) -> bool {
		auto model = std::make_unique<ModelClass>();
		// Initialize 함수는 이미 textureFilename이 nullptr일 경우를 처리하도록 수정되었으므로 완벽하게 호환됩니다.
		if (!model->Initialize(m_D3D->GetDevice(), modelFile, textureFile)) {
			return false;
		}
		m_Models.push_back(std::move(model));
		return true;
		};

	if (!loadModel(L"./data/floor.obj", L"./data/floor.dds")) return false;         // index 0: floor
	if (!loadModel(L"./data/Lighthouse.obj", L"./data/Lighthouse.dds")) return false; // index 1: lighthouse
	if (!loadModel(L"./data/Bridge.obj", L"./data/Bridge.dds")) return false;       // index 2: bridge
	if (!loadModel(L"./data/Boat.obj", L"./data/Boat.dds")) return false;           // index 3: boat
	if (!loadModel(L"./data/streetlight.obj", L"./data/streetlight.dds")) return false; // index 4: streetlight
	if (!loadModel(L"./data/Rock.obj", L"./data/Rock.dds")) return false;           // index 5: rock
	if (!loadModel(L"./data/male.fbx", L"./data/peopleColors.dds")) return false; // index 6 char
	//if (!loadModel(L"./data/character.fbx")) return false;

	/*	m_Models[6]->LoadAnimation(L"./data/male_idle1_200f.fbx", "idle");
	m_Models[6]->LoadAnimation(L"./data/male_running_20f.fbx", "running");
	m_Models[6]->SetAnimationClip("idle"); */

	m_Models[6]->LoadAnimation(L"./data/idle.fbx", "idle");
	m_Models[6]->LoadAnimation(L"./data/running.fbx", "running");
	m_Models[6]->LoadAnimation(L"./data/attack.fbx", "attack");
	m_Models[6]->LoadAnimation(L"./data/die.fbx", "die");
	m_Models[6]->SetAnimationClip("running");



	// --- 2. 씬에 객체 인스턴스 배치 ---
	// 로드된 모델(인덱스)을 사용하여 씬에 여러 개의 인스턴스를 배치합니다.
	m_SceneInstances.push_back({ 0, XMMatrixTranslation(0.0f, -3.0f, 0.0f), false, {0.0f, -3.0f, 0.0f} }); // Floor

	// Lighthouses (3개)
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(0.0f, 0.0f, 50.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Bridges (2개)
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 50.0f) });
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f) });

	// Boats (3개) - 마지막 보트가 움직임
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(-40.0f, 0.0f, 70.0f), false, {-40.0f, 0.0f, 70.0f} });
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f), false, {0.0f, 2.0f, 25.0f} });

	XMVECTOR boatInitialPos = { 40.0f, 0.0f, 50.0f };
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslationFromVector(boatInitialPos), true, boatInitialPos });

	// Streetlights (3개)
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 40.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(6.5f, 2.2f, 30.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 20.0f) });

	// Rocks (3개)
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(0.0f, 1.0f, 50.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Character (1개)
	XMVECTOR charInitialPos = { 0.0f, -1.5f, 10.0f };
	m_SceneInstances.push_back({
		6,
		XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(XMConvertToRadians(180.0f)) * XMMatrixTranslationFromVector(charInitialPos),
		false, // 보트처럼 움직이지 않음
		charInitialPos
		});


	// --- 광원 객체 초기화 ---
	m_Lights.resize(4);
	m_Lights[0] = new LightClass; 
	m_Lights[0]->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f); m_Lights[0]->SetPosition(-10.0f, 8.0f, 15.0f);
	
	m_Lights[1] = new LightClass; 
	m_Lights[1]->SetDiffuseColor(1.0f, 0.5f, 0.0f, 1.0f); m_Lights[1]->SetPosition(10.0f, 8.0f, 35.0f);
	
	m_Lights[2] = new LightClass; 
	m_Lights[2]->SetDiffuseColor(0.5f, 0.5f, 1.0f, 1.0f); m_Lights[2]->SetPosition(-10.0f, 8.0f, 75.0f);
	
	m_Lights[3] = new LightClass; 
	m_Lights[3]->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f); m_Lights[3]->SetPosition(5.0f, 20.0f, 5.0f);

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



	
		if (instance.modelIndex == 6) // 캐릭터 모델인 경우
		{
			XMMATRIX finalWorldMatrix = instance.worldTransform;

			m_LightShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, // 수정된 월드 행렬 전달
				viewMatrix,
				projectionMatrix,
				model->GetTexture(),
				m_Lights,
				model->GetFinalBoneTransforms());
		}
		else // 그 외 모든 정적 모델
		{
			m_StaticShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), m_Lights);
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


