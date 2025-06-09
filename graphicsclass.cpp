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

	// ���� �ؽ�ó �ε� ���� (����)
	auto loadModelSingle = [&](const wchar_t* modelFile, const wchar_t* textureFile = nullptr) -> bool {
		auto model = make_unique<ModelClass>();
		if (!model->Initialize(m_D3D->GetDevice(), modelFile, textureFile)) return false;
		m_Models.push_back(move(model));
		return true;
		};

	// ���� �ؽ�ó �ε� ���� (���� �߰�)
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
	int lighthouseModelIndex = m_Models.size() - 1; // ��� �� �ε��� ����

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




	// --- 2. ���� ��ü �ν��Ͻ� ��ġ ---
	m_SceneInstances.clear();

	// �ε�� ��(�ε���)�� ����Ͽ� ���� ���� ���� �ν��Ͻ��� ��ġ�մϴ�.

	// �� 1]
	m_SceneInstances.push_back({ 5, XMMatrixScaling(2.0f, 0.3f, 1.5f) * XMMatrixTranslation(0.0f, -5.0f, 0.0f) }); // index 5: rock

	// �� 2
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.5f, 0.3f, 1.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslation(100.0f, -5.0f, 100.0f) });

	// �� 3
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.3f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(0.0f)) * XMMatrixTranslation(0.0f, -4.0f, 150.0f) });

	// �� 4
	m_SceneInstances.push_back({ 5, XMMatrixScaling(1.0f, 0.4f, 0.8f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslation(-60.0f, -2.0f, 240.0f) });


	// �ٸ�

	// �ٸ� 1
	m_SceneInstances.push_back({ 2, XMMatrixScaling(5.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(-45.0f)) * XMMatrixTranslation(45.0f, -3.0f, 45.0f) }); // index 2: bridge

	// �ٸ� 2
	m_SceneInstances.push_back({ 2, XMMatrixScaling(3.0f, 2.0f, 3.0f) * XMMatrixRotationY(XMConvertToRadians(30.0f)) * XMMatrixTranslation(27.0f, -1.5f, 140.0f) });


	// ��� ���ε�

	// ��� 1
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.0f, 1.0f, 1.0f) * XMMatrixTranslation(-20.0f, -1.5f, 32.0f) }); // index 1: lighthouse

	// ��� 2
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(120.0f, -4.0f, 130.0f) });

	//��� 3
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(-60.0f, -2.0f, 240.0f) });

	//��� 4
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(4.5f, 4.5f, 4.5f) * XMMatrixTranslation(40.0f, -6.0f, 480.0f) });

	//��� 5
	m_SceneInstances.push_back({ lighthouseModelIndex, XMMatrixScaling(1.5f, 1.5f, 1.5f) * XMMatrixTranslation(10.0f, -4.0f, 600.0f) });

	// ���ε�
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(-10.0f, -2.3f, 24.0f) }); // index 4: streetlight
	m_SceneInstances.push_back({ 4, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixTranslation(15.0f, -2.3f, 27.0f) });


	// --- ĳ���Ϳ� ��Ʈ�� �ڿ������� ��ġ ---

	// [ĳ����] ���� �� ��(�� 2)�� �ٸ� �Ա��� ���� �߾� ���� �ٶ󺸴� ����.
	XMVECTOR charInitialPos = { -28.0f, -1.5f, 18.0f };
	m_SceneInstances.push_back({
		6,
		XMMatrixScaling(0.02f, 0.02f, 0.02f) * XMMatrixRotationY(XMConvertToRadians(45.0f)) * XMMatrixTranslationFromVector(charInitialPos),
		false,
		charInitialPos
		}); // index 6: char (�������� �۰� ����)

	// [�����̴� ��Ʈ] ���� ���̸� �����ϴ� ���.
	XMVECTOR boatInitialPos = { -10.0f, -5.0f, 75.0f };
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(XMConvertToRadians(90.0f)) * XMMatrixTranslationFromVector(boatInitialPos), true, boatInitialPos }); // index 3: boat

	// [������ ��Ʈ] �߾� ��(�� 1) ��ó�� ������ ���.
	m_SceneInstances.push_back({ 3, XMMatrixScaling(1.2f, 1.2f, 1.2f) * XMMatrixRotationY(XMConvertToRadians(100.0f)) * XMMatrixTranslation(40.0f, -5.0f, 80.0f) });

	// [���� floor] �ٴ� ������ �� �⺻ �ٴ�. �ſ� �а� ����ݴϴ�.
	m_SceneInstances.push_back({ 0, XMMatrixScaling(5.0f, 1.0f, 5.0f) * XMMatrixTranslation(0.0f, -6.0f, 0.0f) }); // index 0: floor


	// --- ���� ��ü �ʱ�ȭ ---
	m_Lights.clear(); // ���� ���� ����
	m_Lights.resize(4);

	// 1. ���� ���⼺ ���� (�޺�ó�� �ణ ������ �񽺵���)
	m_Lights[0] = new LightClass();
	m_Lights[0]->SetLightType(LightType::Directional);
	m_Lights[0]->SetDirection(0.5f, -0.7f, 0.5f); // �񽺵��� ����
	m_Lights[0]->SetDiffuseColor(0.8f, 0.8f, 1.0f, 1.0f); // ���� Ǫ����
	m_Lights[0]->SetSpecularColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Lights[0]->SetSpecularPower(64.0f);

	// 2. ��� 1�� ����Ʈ����Ʈ
	m_Lights[1] = new LightClass();
	m_Lights[1]->SetLightType(LightType::Spot);
	m_Lights[1]->SetPosition(40.0f, 64.0f, 480.0f); // ��� �� ����
	m_Lights[1]->SetDirection(0.0f, -0.3f, 1.0f); // �ణ �Ʒ� ������ ����
	m_Lights[1]->SetDiffuseColor(50.0f, 50.0f, 20.0f, 1.0f); // ���� �����
	m_Lights[1]->SetSpecularColor(50.0f, 50.0f, 50.0f, 1.0f);
	m_Lights[1]->SetSpecularPower(256.0f);
	m_Lights[1]->SetSpotlightAngle(15.0f, 25.0f);

	// 3. ���ε� 1�� ����Ʈ����Ʈ
	m_Lights[2] = new LightClass();
	m_Lights[2]->SetLightType(LightType::Spot);
	m_Lights[2]->SetPosition(-10.0f, 5.0f, 24.0f);
	m_Lights[2]->SetDirection(0.0f, -1.0f, 0.0f); // �ٷ� �Ʒ��� ����
	m_Lights[2]->SetDiffuseColor(80.0f, 60.0f, 20.0f, 1.0f);
	m_Lights[2]->SetSpecularColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[2]->SetSpecularPower(128.0f);
	m_Lights[2]->SetSpotlightAngle(40.0f, 60.0f);

	// 4. ���ε� 2�� ����Ʈ����Ʈ
	m_Lights[3] = new LightClass();
	m_Lights[3]->SetLightType(LightType::Spot);
	m_Lights[3]->SetPosition(15.0f, 5.0f, 27.0f);
	m_Lights[3]->SetDirection(0.0f, -1.0f, 0.0f);
	m_Lights[3]->SetDiffuseColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[3]->SetSpecularColor(8.0f, 6.0f, 2.0f, 1.0f);
	m_Lights[3]->SetSpecularPower(128.0f);
	m_Lights[3]->SetSpotlightAngle(40.0f, 60.0f);


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
	if (m_PBRShader) { m_PBRShader->Shutdown(); delete m_PBRShader; m_PBRShader = nullptr; }
	if (m_StaticShader) { m_StaticShader->Shutdown(); delete m_StaticShader; m_StaticShader = nullptr; }
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

	//��� ���� ȸ��
	m_LighthouseRotationAngle += m_LighthouseRotationSpeed * deltaTime;
	if (m_LighthouseRotationAngle > XM_2PI)
	{
		m_LighthouseRotationAngle -= XM_2PI;
	}

	float newDirX = sinf(m_LighthouseRotationAngle);
	float newDirZ = cosf(m_LighthouseRotationAngle);

	if (m_Lights.size() > 1 && m_Lights[1] != nullptr)
	{
		// SetDirection�� ȣ���Ͽ� ������ ������ �ǽð����� ����
		m_Lights[1]->SetDirection(newDirX, -0.3f, newDirZ);
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

		switch (model->GetShaderType())
		{
		case ModelClass::ShaderType::PBR:
			m_PBRShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTextures(), // ���� �ؽ�ó ����
				m_Lights, gameCamera);
			break;

		case ModelClass::ShaderType::Animated:
			m_LightShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), // ���� �ؽ�ó ����
				m_Lights, model->GetFinalBoneTransforms(), gameCamera);
			break;

		case ModelClass::ShaderType::Default:
		default:
			m_StaticShader->Render(m_D3D->GetDeviceContext(), model->GetIndexCount(),
				finalWorldMatrix, viewMatrix, projectionMatrix,
				model->GetTexture(), // ���� �ؽ�ó ����
				m_Lights, gameCamera);
			break;
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


