////////////////////////////////////////////////////////////////////////////////
// Filename: lightclass.h
////////////////////////////////////////////////////////////////////////////////
#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_


//////////////
// INCLUDES //
//////////////
#include <directxmath.h>

using namespace DirectX;

enum class LightType
{
	Directional = 0, // 방향성 조명 (전역)
	Point = 1,       // 점 광원 (현재 시스템과 유사)
	Spot = 2         // 스포트라이트 (등대, 가로등)
};

////////////////////////////////////////////////////////////////////////////////
// Class name: LightClass
////////////////////////////////////////////////////////////////////////////////
class LightClass
{
public:
	LightClass();
	LightClass(const LightClass&);
	~LightClass();

    // 공통 속성 설정
    void SetLightType(LightType type);
    void SetDiffuseColor(float r, float g, float b, float a);
    void SetSpecularColor(float r, float g, float b, float a);
    void SetSpecularPower(float power);
    void SetPosition(float x, float y, float z);
    void SetDirection(float x, float y, float z); // 방향성, 스포트라이트용

    // 스포트라이트 전용 속성 설정
    void SetSpotlightAngle(float innerConeAngle, float outerConeAngle);

    // 속성 가져오기 (셰이더로 전달하기 위함)
    LightType GetLightType() const;
    XMFLOAT4 GetDiffuseColor() const;
    XMFLOAT4 GetSpecularColor() const;
    float GetSpecularPower() const;
    XMFLOAT4 GetPosition() const;
    XMFLOAT3 GetDirection() const;
    float GetInnerConeAngle() const;
    float GetOuterConeAngle() const;

private:
    LightType m_type;
    XMFLOAT4 m_diffuseColor;
    XMFLOAT4 m_specularColor;
    float m_specularPower;

    XMFLOAT4 m_position; // 점, 스포트라이트용
    XMFLOAT3 m_direction; // 방향성, 스포트라이트용

    // 스포트라이트용 각도 (코사인 값으로 저장)
    float m_innerConeCosAngle;
    float m_outerConeCosAngle;
};

#endif