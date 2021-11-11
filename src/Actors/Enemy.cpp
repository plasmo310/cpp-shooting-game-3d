#include "Enemy.h"
#include "EnemyMarker.h"
#include "../Game.h"
#include "../Commons/Mesh.h"
#include "../Components/BoxColliderComponent.h"
#include "../Components/MeshComponent.h"
#include <cmath>

Enemy::Enemy(class Game *game)
: Actor(game)
, mMoveType(STRAIGHT)
, mSpeed(5.0f)
, mShakeWidth(5.0f)
, mTimeCount(0.0f)
, mWaitTime(0.0f)
, mMarker(nullptr)
{
    auto* meshComp = new MeshComponent(this);
    auto* mesh = game->GetRenderer()->GetMesh(game->GetAssetsPath() + "enemy.fbx");
    meshComp->SetMesh(mesh);
    auto* shader = game->GetRenderer()->GetShader(Shader::ShaderType::PHONG);
    meshComp->SetShader(shader);
    mCollider = new BoxColliderComponent(this);
    mCollider->SetObjectAABB(mesh->GetBox());

    // マーカー追加
    mMarker = new EnemyMarker(game);
    mMarker->SetTarget(this);

    // エネミー追加
    game->AddEnemy(this);
}

Enemy::~Enemy()
{
    // エネミー削除
    GetGame()->RemoveEnemy(this);
    // マーカー削除
    mMarker->SetState(EDead);
}

void Enemy::UpdateActor(float deltaTime)
{
    Actor::UpdateActor(deltaTime);

    // 待機時間分待つ
    mTimeCount += deltaTime;
    if (mTimeCount < mWaitTime) return;

    // 位置の更新
    Vector3 pos = GetPosition();
    switch (mMoveType)
    {
        case STRAIGHT:
            pos += mSpeed * deltaTime * GetForward();
            break;
        case SHAKE:
            pos.y = mInitPosition->y + (sinf(mTimeCount / 0.2f) * mShakeWidth);
            pos += mSpeed * deltaTime * GetForward();
            break;
        default:
            break;
    }
    SetPosition(pos);

    // プレイヤーに近づいたら移動を止める
    if ((fabs(pos.x - 0.0f) <= 1.0f)
        && fabs(pos.z - 0.0f) <= 1.0f)
    {
        mSpeed = 0.0f;
        // 喜びの舞い
        SetRotationY(mHappyRotSpeed);
    }
}

void Enemy::ProcessInput(const uint8_t *state, float deltaTime)
{
    Actor::ProcessInput(state, deltaTime);
}

// 上から見た時の角度から位置を設定する
// *正面：0度 右斜め前：30度
void Enemy::SetInitPositionByDegree(float degree)
{
    // Y軸の回転クォータニオン
    Quaternion q(Math::VEC3_UNIT_Y, Math::ToRadians(degree));
    // 正面のベクトルを回転した位置を求める
    Vector3 pos = mAppearDistance * Math::VEC3_UNIT_Z;
    pos = Quaternion::RotateVec(pos, q);
    Actor::SetPosition(pos);
    // 初期位置を保持する
    mInitPosition = new Vector3(pos.x, pos.y, pos.z);
    // 中央を向かせる(270-θ)
    float radian = atan2(pos.z, pos.x);
    SetRotationY(1.5f*Math::Pi - radian);
}