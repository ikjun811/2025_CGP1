////////////////////////////////////////////////////////////////////////////////
// Filename: graphicsclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "graphicsclass.h"


GraphicsClass::GraphicsClass(const GraphicsClass& other)
{
}


GraphicsClass::~GraphicsClass()
{
}


bool GraphicsClass::Initialize(int screenWidth, int screenHeight, HWND hwnd)
{
	bool result;

	XMMATRIX baseViewMatrix;

	// Create the Direct3D object.
	m_D3D = new D3DClass;
	if(!m_D3D)
	{
		return false;
	}

	// Initialize the Direct3D object.
	result = m_D3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if(!result)
	{
		MessageBox(hwnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return false;
	}

	// Create the camera object.
	m_Camera = new CameraClass;
	if(!m_Camera)
	{
		return false;
	}

	// Set the initial position of the camera.
	m_Camera->SetPosition(0.0f, 5.0f, 0.0f);	// for cube
//	m_Camera->SetPosition(0.0f, 0.5f, -3.0f);	// for chair
		
	// Create the model object.
	for (int i = 0; i < 15; ++i)
	{
		m_Model[i] = new ModelClass;
		if (!m_Model[i])
		{
			return false;
		}
	}

	// Initialize the model object.
	//바다
	result = m_Model[0]->Initialize(m_D3D->GetDevice(), L"./data/floor.obj", L"./data/floor.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the floor object.", L"Error", MB_OK);
		return false;
	}

	//등대3
	for (int i = 1; i <= 3; ++i) {
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), L"./data/Lighthouse.obj", L"./data/Lighthouse.dds");
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Lighthouse object.", L"Error", MB_OK);
			return false;
		}
	}

	//다리2
	for (int i = 4; i <= 5; ++i) {
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), L"./data/Bridge.obj", L"./data/Bridge.dds");
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Bridge object.", L"Error", MB_OK);
			return false;
		}
	}

	//보트2
	for (int i = 6; i <= 7; ++i) {
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), L"./data/Boat.obj", L"./data/Boat.dds");
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Boat object.", L"Error", MB_OK);
			return false;
		}
	}

	//가로등3
	for (int i = 8; i <= 10; ++i) {
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), L"./data/streetlight.obj", L"./data/streetlight.dds");
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the streetlight object.", L"Error", MB_OK);
			return false;
		}
	}


	//섬3
	for (int i = 11; i <= 13; ++i) {
		result = m_Model[i]->Initialize(m_D3D->GetDevice(), L"./data/Rock.obj", L"./data/Rock.dds");
		if (!result)
		{
			MessageBox(hwnd, L"Could not initialize the Rock object.", L"Error", MB_OK);
			return false;
		}
	}

	result = m_Model[14]->Initialize(m_D3D->GetDevice(), L"./data/Boat.obj", L"./data/Boat.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the Boat object.", L"Error", MB_OK);
		return false;
	}

	m_Character = new ModelClass;
	if (!m_Character)
	{
		return false;
	}

	result = m_Character->Initialize(m_D3D->GetDevice(), L"./data/character.fbx", L"./data/character_d.dds");
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the character model object.", L"Error", MB_OK);
		return false;
	}

	/*// Create the light shader object.
	m_LightShader = new LightShaderClass;
	if (!m_LightShader)
	{
		return false;
	}

	// Initialize the light shader object.
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the first light object.
	m_Light1 = new LightClass;
	if (!m_Light1)
	{
		return false;
	}

	// Initialize the first light object.
	m_Light1->SetDiffuseColor(1.0f, 0.0f, 0.0f, 1.0f);
	m_Light1->SetPosition(-6.5f, 5.0f, 40.0f);

	// Create the second light object.
	m_Light2 = new LightClass;
	if (!m_Light2)
	{
		return false;
	}

	// Initialize the second light object.
	m_Light2->SetDiffuseColor(0.0f, 1.0f, 0.0f, 1.0f);
	m_Light2->SetPosition(6.5f, 5.0f, 30.0f);

	// Create the third light object.
	m_Light3 = new LightClass;
	if (!m_Light3)
	{
		return false;
	}

	// Initialize the third light object.
	m_Light3->SetDiffuseColor(0.0f, 0.0f, 1.0f, 1.0f);
	m_Light3->SetPosition(-6.5f, 5.0f, 20.0f);
	*/

	// Create the texture shader object.
	m_TextureShader = new TextureShaderClass;
	if (!m_TextureShader)
	{
		return false;
	}

	// Initialize the texture shader object.
	result = m_TextureShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the texture shader object.", L"Error", MB_OK);
		return false;
	}

	// Create the bitmap object.
	m_Bitmap = new BitmapClass;
	if (!m_Bitmap)
	{
		return false;
	}

	// Initialize the bitmap object.
	result = m_Bitmap->Initialize(m_D3D->GetDevice(), screenWidth, screenHeight, L"./data/space.dds", screenWidth, screenHeight);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the bitmap object.", L"Error", MB_OK);
		return false;
	}

	// Initialize a base view matrix with the camera for 2D user interface rendering.
//	m_Camera->SetPosition(0.0f, 0.0f, -1.0f);

	XMVECTOR eye = XMVectorSet(0.0f, 0.0f, -1.0f, 0.0f); // Z축에서 약간 뒤
	XMVECTOR at = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);  // 원점
	XMVECTOR up_vec = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f); // Y축 위
	baseViewMatrix = XMMatrixLookAtLH(eye, at, up_vec);

	// Create the text object.
	m_Text = new TextClass;
	if (!m_Text)
	{
		return false;
	}

	// Initialize the text object.
	result = m_Text->Initialize(m_D3D->GetDevice(), m_D3D->GetDeviceContext(), hwnd, screenWidth, screenHeight, baseViewMatrix);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the text object.", L"Error", MB_OK);
		return false;
	}


	/*// Create the light shader object.
	m_LightShader = new LightShaderClass;
	if (!m_LightShader)
	{
		return false;
	}

	// Initialize the light shader object.
	result = m_LightShader->Initialize(m_D3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the light shader object.", L"Error", MB_OK);
		return false;
	}

	*/


	return true;
}


void GraphicsClass::Shutdown()
{
	// Release the model object.
	for (int i = 0; i < 15; ++i)
	{
		if (m_Model[i])
		{
			m_Model[i]->Shutdown();
			delete m_Model[i];
			m_Model[i] = 0;
		}
	}

	if (m_Character)
	{
		m_Character->Shutdown();
		delete m_Character;
		m_Character = 0;
	}

	// Release the camera object.
	if(m_Camera)
	{
		delete m_Camera;
		m_Camera = 0;
	}

	// Release the D3D object.
	if(m_D3D)
	{
		m_D3D->Shutdown();
		delete m_D3D;
		m_D3D = 0;
	}


	// Release the bitmap object.
	if (m_Bitmap)
	{
		m_Bitmap->Shutdown();
		delete m_Bitmap;
		m_Bitmap = 0;
	}

	// Release the texture shader object.
	if (m_TextureShader)
	{
		m_TextureShader->Shutdown();
		delete m_TextureShader;
		m_TextureShader = 0;
	}

	if (m_Text)
	{
		m_Text->Shutdown();
		delete m_Text;
		m_Text = 0;
	}

	/*	// Release the light shader object.
	if (m_LightShader)
	{
		m_LightShader->Shutdown();
		delete m_LightShader;
		m_LightShader = 0;
	}

	// Release the light objects.
	if (m_Light1)
	{
		delete m_Light1;
		m_Light1 = 0;
	}

	if (m_Light2)
	{
		delete m_Light2;
		m_Light2 = 0;
	}

	if (m_Light3)
	{
		delete m_Light3;
		m_Light3 = 0;
	}
*/

	
	return;
}



bool GraphicsClass::Frame(int fps, int cpu, CameraClass* gameCamera)
{


	bool result;

	float minZPosition = m_ObjectStartPosition - 5.0f;
	float maxZPosition = m_ObjectStartPosition + 5.0f;

	static float rotation = 0.0f;

	// Update the rotation variable each frame.
	rotation += (float)XM_PI * 0.005f;
	if (rotation > 360.0f)
	{
		rotation -= 360.0f;
	}

	if (m_ObjectMovingForward)
	{
		m_ObjectZOffset += m_ObjectSpeed;
		if (m_ObjectZOffset >= maxZPosition)
		{
			m_ObjectMovingForward = false;
		}
	}
	else
	{
		m_ObjectZOffset -= m_ObjectSpeed;
		if (m_ObjectZOffset <= minZPosition)
		{
			m_ObjectMovingForward = true;
		}
	}

	// Set the frames per second.
	result = m_Text->SetFPS(fps, m_D3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	// Set the cpu usage.
	result = m_Text->SetCPU(cpu, m_D3D->GetDeviceContext());
	if (!result)
	{
		return false;
	}

	result = Render(rotation, gameCamera);
	if (!result)
	{
		return false;
	}


	// Set the position of the camera.
	//m_Camera->SetPosition(0.0f, 0.0f, -10.0f);

	return true;
}

bool GraphicsClass::Render(float rotation, CameraClass* gameCamera)
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;
	XMMATRIX scalingMatrix, rotationMatrix;
	XMMATRIX translationMatrix;
	XMFLOAT3 pivotPosition;
	XMMATRIX toOriginMatrix;
	XMMATRIX toPivotMatrix;

	//XMFLOAT4 diffuseColor[4];
	//XMFLOAT4 lightPosition[4];

	bool result;

	/*// Create the diffuse color array from the four light colors.
	diffuseColor[0] = m_Light1->GetDiffuseColor();
	diffuseColor[1] = m_Light2->GetDiffuseColor();
	diffuseColor[2] = m_Light3->GetDiffuseColor();
	diffuseColor[3] = m_Light4->GetDiffuseColor();

	// Create the light position array from the four light positions.
	lightPosition[0] = m_Light1->GetPosition();
	lightPosition[1] = m_Light2->GetPosition();
	lightPosition[2] = m_Light3->GetPosition();
	lightPosition[3] = m_Light4->GetPosition();
	*/
	// Clear the buffers to begin the scene.
	m_D3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f);

	// Generate the view matrix based on the camera's position.
	//m_Camera->Render();
	gameCamera->GetViewMatrix(viewMatrix);

	// Get the world, view, and projection matrices from the camera and d3d objects.
	//m_Camera->GetViewMatrix(viewMatrix);
	m_D3D->GetWorldMatrix(worldMatrix);
	m_D3D->GetProjectionMatrix(projectionMatrix);

	m_D3D->GetOrthoMatrix(orthoMatrix);

	
	// Turn off the Z buffer to begin all 2D rendering.
	m_D3D->TurnZBufferOff();

	XMMATRIX uiWorldMatrix = XMMatrixTranslation(0.0f, 0.0f, 1.0f);

	XMMATRIX uiViewMatrix = XMMatrixIdentity();


	// Put the bitmap vertex and index buffers on the graphics pipeline to prepare them for drawing.
	result = m_Bitmap->Render(m_D3D->GetDeviceContext(), 0, 0);
	if (!result)
	{
		return false;
	}

	// Render the bitmap with the texture shader.
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Bitmap->GetIndexCount(), uiWorldMatrix, uiViewMatrix, orthoMatrix, m_Bitmap->GetTexture());
	if (!result)
	{
		return false;
	}

	m_D3D->TurnOnAlphaBlending();

	// Render the text strings.
	result = m_Text->Render(m_D3D->GetDeviceContext(), worldMatrix, orthoMatrix);
	if (!result)
	{
		return false;
	}

	// Turn off alpha blending after rendering the text.
	m_D3D->TurnOffAlphaBlending();



	// Turn the Z buffer back on now that all 2D rendering has completed.
	m_D3D->TurnZBufferOn();


	// Render the model using the light shader.
	/*result = m_LightShader->Render(m_D3D->GetDeviceContext(), m_Model[14]->GetIndexCount(), worldMatrix, viewMatrix, projectionMatrix,
		m_Model[14]->GetTexture(), diffuseColor, lightPosition);
	if (!result)
	{
		return false;
	}*/

	// 바닥 (floor)
	translationMatrix = XMMatrixTranslation((float)0.0f, -3.0f, 0.0f);
	m_Model[0]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[0]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[0]->GetTexture());
	if (!result) return false;

	//등대 3
	scalingMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	translationMatrix = XMMatrixTranslation((float)0.0f, 0.0f, 50.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[1]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[1]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[1]->GetTexture());
	if (!result) return false;


	scalingMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	translationMatrix = XMMatrixTranslation((float)20.0f, 0.0f, 70.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[2]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[2]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[2]->GetTexture());
	if (!result) return false;


	scalingMatrix = XMMatrixScaling(0.8f, 0.8f, 0.8f);
	translationMatrix = XMMatrixTranslation((float)-20.0f, 0.0f, 80.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[3]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[3]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[3]->GetTexture());
	if (!result) return false;
	
	//다리 2
	scalingMatrix = XMMatrixScaling(3.0f, 1.0f, 4.0f);
	translationMatrix = XMMatrixTranslation((float)0.0f, 2.0f, 50.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(90.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[4]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[4]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[4]->GetTexture());
	if (!result) return false;	
	
	scalingMatrix = XMMatrixScaling(3.0f, 1.0f, 4.0f);
	translationMatrix = XMMatrixTranslation((float)0.0f, 2.0f, 25.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(90.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[5]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[5]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[5]->GetTexture());
	if (!result) return false;

	//보트2

	scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	pivotPosition = { 3.0f, 0.0f, 3.0f };
	rotationMatrix = XMMatrixRotationY(rotation);
	toOriginMatrix = XMMatrixTranslation(-pivotPosition.x, -pivotPosition.y, -pivotPosition.z);
	toPivotMatrix = XMMatrixTranslation(pivotPosition.x, pivotPosition.y, pivotPosition.z);
	translationMatrix = XMMatrixTranslation((float)-40.0f, 0.0f, 70.0f);
	worldMatrix = scalingMatrix * toOriginMatrix * rotationMatrix * toPivotMatrix * translationMatrix;
	m_Model[6]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[6]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[6]->GetTexture());
	if (!result) return false;



	scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	translationMatrix = XMMatrixTranslation((float)40.0f, 0.0f, m_ObjectStartPosition + m_ObjectZOffset);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(0.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[7]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[7]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[7]->GetTexture());
	if (!result) return false;

	scalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);
	translationMatrix = XMMatrixTranslation((float)50.0f, 0.0f, m_ObjectStartPosition + m_ObjectZOffset + 3.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(0.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[14]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[14]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[14]->GetTexture());
	if (!result) return false;

	//가로등3
	scalingMatrix = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	translationMatrix = XMMatrixTranslation((float)-6.5f, 2.2f, 40.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(0.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[8]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[8]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[8]->GetTexture());
	if (!result) return false;

	scalingMatrix = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	translationMatrix = XMMatrixTranslation((float)6.5f, 2.2f, 30.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(180.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[9]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[9]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[9]->GetTexture());
	if (!result) return false;

	scalingMatrix = XMMatrixScaling(3.0f, 3.0f, 3.0f);
	translationMatrix = XMMatrixTranslation((float)-6.5f, 2.2f, 20.0f);
	rotationMatrix = XMMatrixRotationY(XMConvertToRadians(0.0f));
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;
	m_Model[10]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[10]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[10]->GetTexture());
	if (!result) return false;

	//섬3
	scalingMatrix = XMMatrixScaling(0.2f, 0.3f, 0.2f);
	translationMatrix = XMMatrixTranslation((float)0.0f, 1.0f, 50.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[11]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[11]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[11]->GetTexture());
	if (!result) return false;

	scalingMatrix = XMMatrixScaling(0.2f, 0.3f, 0.2f);
	translationMatrix = XMMatrixTranslation((float)20.0f, 0.0f, 70.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[12]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[12]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[12]->GetTexture());
	if (!result) return false;

	scalingMatrix = XMMatrixScaling(0.2f, 0.3f, 0.2f);
	translationMatrix = XMMatrixTranslation((float)-20.0f, 0.0f, 80.0f);
	worldMatrix = scalingMatrix * translationMatrix;
	m_Model[13]->Render(m_D3D->GetDeviceContext());
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Model[13]->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Model[13]->GetTexture());
	if (!result) return false;

	 scalingMatrix = XMMatrixScaling(0.1f, 0.1f, 0.1f); // 크기 조절 (필요 시)
	 rotationMatrix = XMMatrixRotationY(XMConvertToRadians(180.0f)); // 180도 회전하여 카메라를 보게 함
	 translationMatrix = XMMatrixTranslation(0.0f, -1.5f, 10.0f); // 월드 좌표 (0, -1.5, 10)에 배치

	// 최종 월드 행렬 계산
	m_D3D->GetWorldMatrix(worldMatrix); // 기본 월드 행렬 가져오기
	worldMatrix = scalingMatrix * rotationMatrix * translationMatrix;

	// 캐릭터 모델의 버퍼를 파이프라인에 설정
	m_Character->Render(m_D3D->GetDeviceContext());

	// 텍스처 셰이더를 사용하여 캐릭터 모델 렌더링
	result = m_TextureShader->Render(m_D3D->GetDeviceContext(), m_Character->GetIndexCount(),
		worldMatrix, viewMatrix, projectionMatrix, m_Character->GetTexture());
	if (!result)
	{
		return false;
	}

	// Present the rendered scene to the screen.
	m_D3D->EndScene();

	return true;
}


