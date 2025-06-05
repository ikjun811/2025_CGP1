////////////////////////////////////////////////////////////////////////////////
// Filename: cameraclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


//////////////
// INCLUDES //
//////////////
#include <directxmath.h>

#include "AlignedAllocationPolicy.h"

#include "inputclass.h"

using namespace DirectX;

////////////////////////////////////////////////////////////////////////////////
// Class name: CameraClass
////////////////////////////////////////////////////////////////////////////////
class CameraClass : public AlignedAllocationPolicy<16>
{
public:
	CameraClass();
	CameraClass(const CameraClass&);
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition() const; 
	XMFLOAT3 GetRotation() const; // Yaw, Pitch, Roll ¹ÝÈ¯

	void Render();
	void GetViewMatrix(XMMATRIX&) const;

	void HandleMovement(const InputClass& input, float frameTime);

private:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMMATRIX m_viewMatrix;

	float m_moveSpeed;
};

#endif