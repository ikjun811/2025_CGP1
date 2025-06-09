////////////////////////////////////////////////////////////////////////////////
// Filename: lightclass.cpp
////////////////////////////////////////////////////////////////////////////////
#include "lightclass.h"
#include <cmath>

LightClass::LightClass()
{
	m_type = LightType::Directional;
	m_diffuseColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_specularColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	m_specularPower = 32.0f;
	m_position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	m_direction = XMFLOAT3(0.0f, -1.0f, 0.0f);
	SetSpotlightAngle(45.0f, 60.0f); // 기본값 설정

}


LightClass::LightClass(const LightClass& other)
{
}


LightClass::~LightClass()
{
}


// --- Setters ---
void LightClass::SetLightType(LightType type) { m_type = type; }
void LightClass::SetDiffuseColor(float r, float g, float b, float a) { m_diffuseColor = XMFLOAT4(r, g, b, a); }
void LightClass::SetSpecularColor(float r, float g, float b, float a) { m_specularColor = XMFLOAT4(r, g, b, a); }
void LightClass::SetSpecularPower(float power) { m_specularPower = power; }
void LightClass::SetPosition(float x, float y, float z) { m_position = XMFLOAT4(x, y, z, 1.0f); }
void LightClass::SetDirection(float x, float y, float z) { m_direction = XMFLOAT3(x, y, z); }

void LightClass::SetSpotlightAngle(float innerConeAngle, float outerConeAngle)
{
	// 각도를 라디안으로 변환 후 코사인 값을 저장
	m_innerConeCosAngle = cosf(XMConvertToRadians(innerConeAngle / 2.0f));
	m_outerConeCosAngle = cosf(XMConvertToRadians(outerConeAngle / 2.0f));
}

// --- Getters ---
LightType LightClass::GetLightType() const { return m_type; }
XMFLOAT4 LightClass::GetDiffuseColor() const { return m_diffuseColor; }
XMFLOAT4 LightClass::GetSpecularColor() const { return m_specularColor; }
float LightClass::GetSpecularPower() const { return m_specularPower; }
XMFLOAT4 LightClass::GetPosition() const { return m_position; }
XMFLOAT3 LightClass::GetDirection() const { return m_direction; }
float LightClass::GetInnerConeAngle() const { return m_innerConeCosAngle; }
float LightClass::GetOuterConeAngle() const { return m_outerConeCosAngle; }