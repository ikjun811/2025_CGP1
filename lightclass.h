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
	Directional = 0, // ���⼺ ���� (����)
	Point = 1,       // �� ���� (���� �ý��۰� ����)
	Spot = 2         // ����Ʈ����Ʈ (���, ���ε�)
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

    // ���� �Ӽ� ����
    void SetLightType(LightType type);
    void SetDiffuseColor(float r, float g, float b, float a);
    void SetSpecularColor(float r, float g, float b, float a);
    void SetSpecularPower(float power);
    void SetPosition(float x, float y, float z);
    void SetDirection(float x, float y, float z); // ���⼺, ����Ʈ����Ʈ��

    // ����Ʈ����Ʈ ���� �Ӽ� ����
    void SetSpotlightAngle(float innerConeAngle, float outerConeAngle);

    // �Ӽ� �������� (���̴��� �����ϱ� ����)
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

    XMFLOAT4 m_position; // ��, ����Ʈ����Ʈ��
    XMFLOAT3 m_direction; // ���⼺, ����Ʈ����Ʈ��

    // ����Ʈ����Ʈ�� ���� (�ڻ��� ������ ����)
    float m_innerConeCosAngle;
    float m_outerConeCosAngle;
};

#endif