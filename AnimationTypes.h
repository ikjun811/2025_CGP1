#pragma once

#include <string>
#include <vector>
#include <map>
#include <DirectXMath.h>

using namespace DirectX;

// 스키닝 정보를 포함하도록 확장된 정점 구조체
struct SkinnedVertex
{
    XMFLOAT3 Position;
    XMFLOAT2 TexCoord;
    XMFLOAT3 Normal;
    XMFLOAT3 Tangent; // 노멀 매핑 등을 위해 추가 (선택 사항)

    // 스키닝 데이터
    XMFLOAT4 BoneWeights; // 각 뼈의 가중치
    XMUINT4  BoneIDs;     // 영향을 주는 뼈의 ID
};

// 뼈(Bone/Joint) 하나에 대한 정보
struct BoneInfo
{
    int id;                      // 뼈의 고유 ID (0부터 시작)
    std::string name;            // 뼈의 이름
    XMMATRIX inverseBindPose;    // T-포즈(기본 자세)를 기준으로 한 역행렬
};

// 애니메이션의 특정 시간 지점(Keyframe)에 대한 데이터
template<typename T>
struct Keyframe
{
    float timePos; // 키프레임의 시간 (초)
    T value;       // 해당 시간의 값 (위치: XMFLOAT3, 회전: XMFLOAT4, 크기: XMFLOAT3)
};

// 하나의 뼈에 대한 모든 키프레임 채널
struct BoneAnimation
{
    std::string boneName;
    std::vector<Keyframe<XMFLOAT3>> positionKeys;
    std::vector<Keyframe<XMFLOAT4>> rotationKeys;
    std::vector<Keyframe<XMFLOAT3>> scaleKeys;
};

// 하나의 애니메이션 클립 (예: "Idle", "Running")
struct AnimationClip
{
    std::string name;
    float duration;          // 애니메이션 총 길이 (초)
    float ticksPerSecond;    // 초당 틱 수 (프레임 속도)
    std::vector<BoneAnimation> boneAnimations;
};

// 뼈의 계층 구조를 나타내는 노드
struct BoneNode
{
    std::string name;
    XMMATRIX transformation; // 부모 노드에 대한 상대적 변환
    std::vector<BoneNode> children;
};
