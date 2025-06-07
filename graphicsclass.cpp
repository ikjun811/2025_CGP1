////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass()
{
	m_D3D = nullptr;
	m_TextureShader = nullptr;
	m_LightShader = nullptr;
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

	auto loadModel = [&](const wchar_t* modelFile, const wchar_t* textureFile) -> bool {
		auto model = std::make_unique<ModelClass>();
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
	if (!loadModel(L"./data/character.fbx", L"./data/character_d.dds")) return false; // index 6: character

	// --- 2. 씬에 객체 인스턴스 배치 ---
	// 로드된 모델(인덱스)을 사용하여 씬에 여러 개의 인스턴스를 배치합니다.
	m_SceneInstances.push_back({ 0, XMMatrixTranslation(0.0f, -3.0f, 0.0f) }); // Floor

	// Lighthouses (3개)
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(0.0f, 0.0f, 50.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Bridges (2개)
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 50.0f) });
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f) });

	// Boats (3개) - 마지막 보트가 움직임
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(-40.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f) }); // 다리 위의 보트
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(40.0f, 0.0f, 50.0f), true }); // 움직이는 보트

	// Streetlights (3개)
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 40.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(6.5f, 2.2f, 30.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 20.0f) });

	// Rocks (3개)
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(0.0f, 1.0f, 50.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Character (1개)
	m_SceneInstances.push_back({ 6, XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(XMConvertToRadians(180.0f)) * XMMatrixTranslation(0.0f, -1.5f, 10.0f) });

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
	if (!m_Bitmap || !m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/space.dds", 200, 200)) return false; // 비트맵 크기 확인 필요 (원본 코드 참고)

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
	if (m_LightShader) { m_LightShader->Shutdown(); delete m_LightShader; m_LightShader = nullptr; }
	if (m_TextureShader) { m_TextureShader->Shutdown(); delete m_TextureShader; m_TextureShader = nullptr; }
	m_Models.clear(); // unique_ptr가 모든 ModelClass 메모리 자동 해제
	m_SceneInstances.clear();
	if (m_D3D) { m_D3D->Shutdown(); delete m_D3D; m_D3D = nullptr; }
}



bool GraphicsClass::Frame(int fps, int cpu, CameraClass* gameCamera)
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
	
			instance.worldTransform = XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(40.0f, 0.0f, 50.0f + m_BoatZOffset);
		}
	}

	if (!m_Text->SetFPS(fps, m_D3D->GetDeviceContext())) return false;
	if (!m_Text->SetCPU(cpu, m_D3D->GetDeviceContext())) return false;

	return Render(gameCamera);
}

bool GraphicsClass::Render(CameraClass* gameCamera)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	gameCamera->GetViewMatrix(viewMatrix); // 3D 씬을 위한 뷰 행렬
	m_D3D->GetWorldMatrix(worldMatrix); // 기본 월드 행렬
	m_D3D->GetProjectionMatrix(projectionMatrix);
	m_D3D->GetOrthoMatrix(orthoMatrix);

	// 1. 씬 버퍼 클리어
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	m_D3D->TurnZBufferOff();
	{
		XMMATRIX uiViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		// 비트맵 렌더링
		m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
		
	}
	m_D3D->TurnZBufferOn();
	
	//2 모델 렌더링
	for (const auto& instance : m_SceneInstances)
	{
		ModelClass* model = m_Models[instance.modelIndex].get();
		model->Render(m_D3D->GetDeviceContext());
		m_LightShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
			instance.worldTransform, viewMatrix, projectionMatrix, model->GetTexture(), m_Lights);
	}

	// 3. 2D UI 렌더링 (Z-버퍼 끄기)
	m_D3D->TurnZBufferOff();
	{
		// 2D UI를 위한 뷰 행렬은 단위 행렬(Identity) 또는 고정된 카메라 뷰를 사용합니다.
		// 여기서는 TextClass 초기화에 사용했던 baseViewMatrix를 다시 만듭니다.
		XMMATRIX uiViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

		// 비트맵 렌더링
		m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, uiViewMatrix, orthoMatrix, m_Bitmap->GetTexture());

		// 텍스트 렌더링
		m_D3D->TurnOnAlphaBlending();
		m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix); // TextClass는 내부적으로 뷰 행렬을 관리하므로 전달 안 함
		m_D3D->TurnOffAlphaBlending();
	}
	m_D3D->TurnZBufferOn();

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}


