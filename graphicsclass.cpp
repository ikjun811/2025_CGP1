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
		// Initialize �Լ��� �̹� textureFilename�� nullptr�� ��츦 ó���ϵ��� �����Ǿ����Ƿ� �Ϻ��ϰ� ȣȯ�˴ϴ�.
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



	// --- 2. ���� ��ü �ν��Ͻ� ��ġ ---
	// �ε�� ��(�ε���)�� ����Ͽ� ���� ���� ���� �ν��Ͻ��� ��ġ�մϴ�.
	m_SceneInstances.push_back({ 0, XMMatrixTranslation(0.0f, -3.0f, 0.0f), false, {0.0f, -3.0f, 0.0f} }); // Floor

	// Lighthouses (3��)
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(0.0f, 0.0f, 50.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 1, XMMatrixScaling(0.8f, 0.8f, 0.8f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Bridges (2��)
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 50.0f) });
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 1.0f, 4.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f) });

	// Boats (3��) - ������ ��Ʈ�� ������
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(-40.0f, 0.0f, 70.0f), false, {-40.0f, 0.0f, 70.0f} });
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(0.0f, 2.0f, 25.0f), false, {0.0f, 2.0f, 25.0f} });

	XMVECTOR boatInitialPos = { 40.0f, 0.0f, 50.0f };
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslationFromVector(boatInitialPos), true, boatInitialPos });

	// Streetlights (3��)
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 40.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(6.5f, 2.2f, 30.0f) });
	m_SceneInstances.push_back({ 4, XMMatrixScaling(3.0f, 3.0f, 3.0f) * XMMatrixTranslation(-6.5f, 2.2f, 20.0f) });

	// Rocks (3��)
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(0.0f, 1.0f, 50.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(20.0f, 0.0f, 70.0f) });
	m_SceneInstances.push_back({ 5, XMMatrixScaling(0.2f, 0.3f, 0.2f) * XMMatrixTranslation(-20.0f, 0.0f, 80.0f) });

	// Character (1��)
	XMVECTOR charInitialPos = { 0.0f, -1.5f, 10.0f };
	m_SceneInstances.push_back({
		6,
		XMMatrixScaling(0.1f, 0.1f, 0.1f) * XMMatrixRotationY(XMConvertToRadians(180.0f)) * XMMatrixTranslationFromVector(charInitialPos),
		false, // ��Ʈó�� �������� ����
		charInitialPos
		});


	// --- ���� ��ü �ʱ�ȭ ---
	m_Lights.resize(4);
	m_Lights[0] = new LightClass; 
	m_Lights[0]->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f); m_Lights[0]->SetPosition(-10.0f, 8.0f, 15.0f);
	
	m_Lights[1] = new LightClass; 
	m_Lights[1]->SetDiffuseColor(1.0f, 0.5f, 0.0f, 1.0f); m_Lights[1]->SetPosition(10.0f, 8.0f, 35.0f);
	
	m_Lights[2] = new LightClass; 
	m_Lights[2]->SetDiffuseColor(0.5f, 0.5f, 1.0f, 1.0f); m_Lights[2]->SetPosition(-10.0f, 8.0f, 75.0f);
	
	m_Lights[3] = new LightClass; 
	m_Lights[3]->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f); m_Lights[3]->SetPosition(5.0f, 20.0f, 5.0f);

	// --- UI ��ü �ʱ�ȭ ---
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
	m_Models.clear(); // unique_ptr�� ��� ModelClass �޸� �ڵ� ����
	m_SceneInstances.clear();
	if (m_D3D) { m_D3D->Shutdown(); delete m_D3D; m_D3D = nullptr; }
}



bool GraphicsClass::Frame(int fps, int cpu, CameraClass* gameCamera, float deltaTime)
{
	// ��Ʈ ���� � ������Ʈ
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

	// �ִϸ��̼��� ����� ��ü�� ���� ����� ������Ʈ�մϴ�.
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
			// 3. ���� ���� ����� �����մϴ�.
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

	// 1. ������ ����
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	gameCamera->GetViewMatrix(viewMatrix);      // 3D ���� ���� �� ���
	m_D3D->GetWorldMatrix(worldMatrix);         // �⺻ ���� ���
	m_D3D->GetProjectionMatrix(projectionMatrix); // 3D ���� ���� ���� ���
	m_D3D->GetOrthoMatrix(orthoMatrix);         // 2D �������� ���� ���� ���

	uiViewMatrix = XMMatrixLookAtLH(XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));

	// 2. ��� ��Ʈ�� ������ (Z-���� ����)
	// ����� ���̿� ������� ���� ���� �׷����� �մϴ�.
	m_D3D->TurnZBufferOff();
	{
		// ��Ʈ���� ���ؽ�/�ε��� ���۸� Ȱ��ȭ�ϰ� ��ġ�� �����մϴ�.
		m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
		// �ؽ�ó ���̴��� ����� ��Ʈ���� ������ �׸��ϴ�.
		m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), worldMatrix, uiViewMatrix, orthoMatrix, m_Bitmap->GetTexture());
	}
	m_D3D->TurnZBufferOn(); // 3D ���� �׸��� ���� Z-���۸� �ٽ� �մϴ�.
	
	// 3. �� ������
	for (const auto& instance : m_SceneInstances)
	{
		ModelClass* model = m_Models[instance.modelIndex].get();
		model->Render(m_D3D->GetDeviceContext());

		XMMATRIX finalWorldMatrix = instance.worldTransform;



	
		if (instance.modelIndex == 6) // ĳ���� ���� ���
		{
			XMMATRIX finalWorldMatrix = instance.worldTransform;

			m_LightShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, // ������ ���� ��� ����
				viewMatrix,
				projectionMatrix,
				model->GetTexture(),
				m_Lights,
				model->GetFinalBoneTransforms());
		}
		else // �� �� ��� ���� ��
		{
			m_StaticShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), m_Lights);
		}
	}

	// 4. 2D UI ������ (Z-���� ����)
	m_D3D->TurnZBufferOff();
	{
		// �ؽ�Ʈ ������
		m_D3D->TurnOnAlphaBlending();
		m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
		m_D3D->TurnOffAlphaBlending();
	}
	m_D3D->TurnZBufferOn();

	// 5. ������ ����
	m_D3D->EndScene();

	return true;
}


