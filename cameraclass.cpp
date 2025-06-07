////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "cameraclass.h"


CameraClass::CameraClass()
{
	m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f); // x: pitch, y: yaw, z: roll

	m_moveSpeed = 0.1f;
}


CameraClass::CameraClass(const CameraClass& other)
{
}


CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
}

void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotation.x = x; // Pitch
	m_rotation.y = y; // Yaw
	m_rotation.z = z; // Roll
}



XMFLOAT3 CameraClass::GetPosition() const
{
	return m_position;
}

XMFLOAT3 CameraClass::GetRotation() const
{
	return m_rotation;
}

// This uses the position and rotation of the camera to build and to update the view matrix.
void CameraClass::Render()
{
	XMVECTOR up, position, lookAtTarget;
	XMMATRIX rotationMatrix;

	// ������ �������� ��ȯ
	float pitch = m_rotation.x * 0.0174532925f;
	float yaw = m_rotation.y * 0.0174532925f;
	float roll = m_rotation.z * 0.0174532925f;

	// ȸ�� ��� ���� (Roll, Pitch, Yaw ���� �Ǵ� Pitch, Yaw, Roll ���� - ������ P,Y,R)
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);


	up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// ȸ���� '�ٶ󺸴� ����' ���� ���
	XMVECTOR lookDirection = XMVector3TransformCoord(defaultForward, rotationMatrix);

	// ī�޶� ��ġ ���� �ε�
	position = XMLoadFloat3(&m_position);

	// ī�޶� �ٶ󺸴� ���� ��ǥ ���� ���
	lookAtTarget = position + lookDirection;

	// ���������� �ùٸ� ����� �� ����� �����մϴ�.
	m_viewMatrix = XMMatrixLookAtLH(position, lookAtTarget, up);
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix) const
{
	viewMatrix = m_viewMatrix;
}

void CameraClass::HandleMovement(const InputClass& input, float frameTime)
{
	long mouseDeltaX, mouseDeltaY;
	input.GetMouseDelta(mouseDeltaX, mouseDeltaY);


	// ���콺 �Է����� Yaw, Pitch ������Ʈ (�������� ���� ���� �Ǵ� ������ ���� �� Render���� ��ȯ)
	// ���� �ڵ��� ����(0.001f) ���
	m_rotation.y += (float)mouseDeltaX * 0.1f; // Yaw (�¿� ȸ��)
	m_rotation.x += (float)mouseDeltaY * 0.1f; // Pitch (���� ȸ��)

	// Pitch �� ���� (��: -89�� ~ +89��)
	
	if (m_rotation.x > 89.0f)
	{
		m_rotation.x = 89.0f;
	}
	if (m_rotation.x < -89.0f)
	{
		m_rotation.x = -89.0f;
	}
	


	// Ű���� �Է����� �̵� ó��
	float speed = m_moveSpeed * frameTime;
	float moveForward = 0.0f;
	float moveRight = 0.0f;

	// DIK_ �ڵ�� DirectInput ����� ���ǵǾ� �ֽ��ϴ�.
	if (input.IsKeyDown(DIK_W)) { moveForward += speed; }
	if (input.IsKeyDown(DIK_S)) { moveForward -= speed; }
	if (input.IsKeyDown(DIK_A)) { moveRight -= speed; }
	if (input.IsKeyDown(DIK_D)) { moveRight += speed; }
	if (input.IsKeyDown(DIK_SPACE)) { m_position.y += speed; }
	if (input.IsKeyDown(DIK_LCONTROL) || input.IsKeyDown(DIK_C)) { m_position.y -= speed; }
	



	// ���� ī�޶��� Yaw ȸ������ ����� ���� �� ���
	float yawRad = m_rotation.y * 0.0174532925f;
	XMMATRIX rotationYMatrix = XMMatrixRotationY(yawRad);

	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR camForwardVec = XMVector3TransformCoord(defaultForward, rotationYMatrix);
	XMVECTOR camRightVec = XMVector3TransformCoord(defaultRight, rotationYMatrix);

	// ��ġ ������Ʈ
	XMVECTOR positionVec = XMLoadFloat3(&m_position);
	positionVec += camForwardVec * moveForward;
	positionVec += camRightVec * moveRight;
	XMStoreFloat3(&m_position, positionVec);

	// ī�޶� ��ġ�� ����Ǿ����Ƿ� Render()�� ȣ���Ͽ� �� ����� �ٽ� ����ؾ� ��
	// �Ǵ�, �� �Լ��� Camera::Render() ���� ȣ��ȴٸ� Render()���� �ڵ����� �ݿ���
}