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

	// 각도를 라디안으로 변환
	float pitch = m_rotation.x * 0.0174532925f; // m_rotation.x가 pitch
	float yaw = m_rotation.y * 0.0174532925f; // m_rotation.y가 yaw
	float roll = m_rotation.z * 0.0174532925f; // m_rotation.z가 roll (FPS에서는 보통 0)

	// 회전 행렬 생성 (Roll, Pitch, Yaw 순서 또는 Pitch, Yaw, Roll 순서 - 예제는 P,Y,R)
	rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// 기본 전방 벡터 (Z축 양의 방향)
	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	// 기본 위쪽 벡터 (Y축 양의 방향)
	XMVECTOR defaultUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	// 회전된 '바라보는 방향' 벡터 계산
	XMVECTOR lookDirection = XMVector3TransformCoord(defaultForward, rotationMatrix);
	// 회전된 '위쪽' 벡터 계산
	up = XMVector3TransformCoord(defaultUp, rotationMatrix);


	// 카메라 위치 벡터 로드
	position = XMLoadFloat3(&m_position);

	// 카메라가 바라보는 최종 목표 지점 계산
	lookAtTarget = position + lookDirection;

	// 뷰 행렬 생성
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

	// 마우스 입력으로 Yaw, Pitch 업데이트 (라디안으로 직접 변경 또는 각도로 변경 후 Render에서 변환)
	// 예제 코드의 감도(0.001f) 사용
	m_rotation.y += (float)mouseDeltaX * 0.02f; // Yaw (좌우 회전)
	m_rotation.x += (float)mouseDeltaY * 0.02f; // Pitch (상하 회전)

	// Pitch 값 제한 (예: -89도 ~ +89도)
	// 0.0174532925f는 1도를 라디안으로 변환하는 값. 1.55334f는 약 89도
	// m_rotation이 도(degree) 단위라면:
	if (m_rotation.x > 89.0f) m_rotation.x = 89.0f;
	if (m_rotation.x < -89.0f) m_rotation.x = -89.0f;
	// m_rotation이 라디안 단위라면:
	// if(m_rotation.x > 1.55f) m_rotation.x = 1.55f;  // 약 89도
	// if(m_rotation.x < -1.55f) m_rotation.x = -1.55f; // 약 -89도


	// 키보드 입력으로 이동 처리
	float speed = m_moveSpeed * frameTime;
	float moveForward = 0.0f;
	float moveRight = 0.0f;

	// DIK_ 코드는 DirectInput 헤더에 정의되어 있습니다.
	if (input.IsKeyDown(DIK_W)) // 전진
	{
		moveForward += speed;
	}
	if (input.IsKeyDown(DIK_S)) // 후진
	{
		moveForward -= speed;
	}
	if (input.IsKeyDown(DIK_A)) // 좌로 이동 (Strafe Left)
	{
		moveRight -= speed;
	}
	if (input.IsKeyDown(DIK_D)) // 우로 이동 (Strafe Right)
	{
		moveRight += speed;
	}

	
		// (선택) 위/아래 이동 (예: DIK_SPACE, DIK_LCONTROL)
	if (input.IsKeyDown(DIK_SPACE))
	{
		m_position.y += speed;
	}
	if (input.IsKeyDown(DIK_LCONTROL) || input.IsKeyDown(DIK_C)) // 보통 Ctrl 이나 C
	{
		m_position.y -= speed;
	}
	



	// 현재 카메라의 Yaw 회전만을 고려한 로컬 축 계산 (Pitch는 이동 방향에 영향 X)
	float yawRad = m_rotation.y * 0.0174532925f;
	XMMATRIX rotationYMatrix = XMMatrixRotationY(yawRad);

	XMVECTOR defaultForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	XMVECTOR defaultRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR camForwardVec = XMVector3TransformCoord(defaultForward, rotationYMatrix);
	XMVECTOR camRightVec = XMVector3TransformCoord(defaultRight, rotationYMatrix);

	// 위치 업데이트
	XMVECTOR positionVec = XMLoadFloat3(&m_position);
	positionVec += camForwardVec * moveForward;
	positionVec += camRightVec * moveRight;
	XMStoreFloat3(&m_position, positionVec);

	// 카메라 위치가 변경되었으므로 Render()를 호출하여 뷰 행렬을 다시 계산해야 함
	// 또는, 이 함수가 Camera::Render() 전에 호출된다면 Render()에서 자동으로 반영됨
}