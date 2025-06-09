#pragma once

#include <string>
#include <vector>
#include <map>
#include <DirectXMath.h>

using namespace DirectX;

// ��Ű�� ������ �����ϵ��� Ȯ��� ���� ����ü
struct SkinnedVertex
{
    XMFLOAT3 Position;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Normal;
    XMFLOAT3 Tangent; // ��� ���� ���� ���� �߰� (���� ����)

    // ��Ű�� ������
    XMFLOAT4 BoneWeights; // �� ���� ����ġ
    XMUINT4  BoneIDs;     // ������ �ִ� ���� ID
};

// ��(Bone/Joint) �ϳ��� ���� ����
struct BoneInfo
{
    int id;                      // ���� ���� ID (0���� ����)
    std::string name;            // ���� �̸�
    XMMATRIX inverseBindPose;    // T-����(�⺻ �ڼ�)�� �������� �� �����
};

// �ִϸ��̼��� Ư�� �ð� ����(Keyframe)�� ���� ������
template<typename T>
struct Keyframe
{
    float timePos; // Ű�������� �ð� (��)
    T value;       // �ش� �ð��� �� (��ġ: XMFLOAT3, ȸ��: XMFLOAT4, ũ��: XMFLOAT3)
};

// �ϳ��� ���� ���� ��� Ű������ ä��
struct BoneAnimation
{
    std::string boneName;
    std::vector<Keyframe<XMFLOAT3>> positionKeys;
    std::vector<Keyframe<XMFLOAT4>> rotationKeys;
    std::vector<Keyframe<XMFLOAT3>> scaleKeys;
};

// �ϳ��� �ִϸ��̼� Ŭ�� (��: "Idle", "Running")
struct AnimationClip
{
    std::string name;
    float duration;          // �ִϸ��̼� �� ���� (��)
    float ticksPerSecond;    // �ʴ� ƽ �� (������ �ӵ�)
    std::vector<BoneAnimation> boneAnimations;
};

// ���� ���� ������ ��Ÿ���� ���
struct BoneNode
{
    std::string name;
    XMMATRIX transformation; // �θ� ��忡 ���� ����� ��ȯ
    std::vector<BoneNode> children;
};
