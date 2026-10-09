#include "GameContext.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "raylib.h"

void GameContext::Init()
{
    // カメラコントローラーの初期化（ウィンドウ生成後に呼ばれ、PlayerCameraとカーソル非表示を確定させる）
    cameraController_.Init();
	player_.Init();
    stage_.Init();
    PlaceCharacters();
}

void GameContext::Reset()
{
	player_.Reset();
    stage_.Reset();
    PlaceCharacters();
}

void GameContext::PlaceCharacters()
{
    // モデルの原点が床とは限らないため、実際の床から開始位置を求める。
    player_.SetPosition(stage_.FindGroundPosition({ 0.0f, 0.0f, 0.0f }));
    enemyManager_.Clear();
    enemyManager_.Spawn(stage_.FindGroundPosition({ 0.9f, 0.0f, 2.5f }), {});
    paladinPosition_ = stage_.FindGroundPosition({ 0.0f, 0.0f, 3.0f });
    cameraController_.GetPlayerCamera().SetPlayerPosition(player_.GetPosition());
    cameraController_.GetPlayerCamera().Update();
}

void GameContext::Update(float deltaTime)
{
    // 1. カメラの向きを取得
    Vector3 forward = cameraController_.GetPlayerCamera().GetForwardVector();
    Vector3 right = cameraController_.GetPlayerCamera().GetRightVector();

    // 2. プレイヤーの移動（ステージとの当たり判定を含めて更新）
    player_.Update(deltaTime, stage_, forward, right);

    // 3. 敵の更新
    enemyManager_.Update(deltaTime, player_, stage_);
    enemyManager_.RemoveExpired();


    int animCount = 0;
    ModelAnimation* anims = ResourceManager::GetInstance().GetModelAnimations(ResourceKeys::Model_Paladin, &animCount);

    if (anims != nullptr && animCount > 0)
    {
        Model& paladinModel = ResourceManager::GetInstance().GetModelRef(ResourceKeys::Model_Paladin);

        animFrame_++;
        if (animFrame_ >= anims[animIndex_].frameCount) {
            animFrame_ = 0;
        }

        UpdateModelAnimation(paladinModel, anims[animIndex_], animFrame_);
    }

    // 4. カメラを移動後のプレイヤーに追従させて更新
    cameraController_.GetPlayerCamera().SetPlayerPosition(player_.GetPosition());
    cameraController_.Update();
}

void GameContext::Draw() const
{
    BeginMode3D(cameraController_.GetActiveRaylibCamera());

    DrawGrid(20, 1.0f);

    stage_.Draw();
    player_.Draw();
    enemyManager_.Draw();
    Model& paladinModel = ResourceManager::GetInstance().GetModelRef(ResourceKeys::Model_Paladin);
    DrawModel(paladinModel, paladinPosition_, 1.0f, WHITE);

    EndMode3D();
}

void GameContext::End()
{
    enemyManager_.Clear();
    stage_.End();
}