#include "GameContext.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "raylib.h"
#include "CameraController.h"

void GameContext::Init()
{
    cameraController_.Init();
    player_.Init();
    stage_.Init();
    enemyManager_.Clear();
    enemyManager_.Spawn({ 0.0f, 0.0f, 3.0f }, demo::EnemyConfig{});
}

void GameContext::Reset()
{
    player_.Reset();
    stage_.Reset();
    enemyManager_.Clear();
    enemyManager_.Spawn({ 0.0f, 0.0f, 3.0f }, demo::EnemyConfig{});
}

void GameContext::Update(float deltaTime)
{
    bool isPlayerCameraActive = (cameraController_.GetActiveType() == CameraType::Player);

    // 1. プレイヤーの移動入力更新（プレイヤーカメラ使用時のみ移動入力受付）
    if (isPlayerCameraActive)
    {
        Vector3 forward = cameraController_.GetPlayerCamera().GetForwardVector();
        Vector3 right = cameraController_.GetPlayerCamera().GetRightVector();
        player_.Update(deltaTime, stage_, forward, right);
    }

    // 2. ワールドオブジェクト（ステージ・敵）の更新
    stage_.Update(deltaTime);
    enemyManager_.Update(deltaTime, player_, stage_);
    enemyManager_.RemoveExpired();

    // パラディンモデルのアニメーション更新
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

    // 3. ギズモ編集結果も含め、常に最新の Player 位置を PlayerCamera へ同期
    cameraController_.GetPlayerCamera().SetPlayerPosition(player_.GetPosition());

    // 4. カメラコントローラーの更新
    cameraController_.Update();
}

void GameContext::Draw() const
{
    BeginMode3D(cameraController_.GetActiveRaylibCamera());

    DrawGrid(20, 1.0f);
    stage_.Draw();

    // Player Camera がアクティブな時だけカメラポインタを渡し、一人称用に追従させる
    const PlayerCamera* activePlayerCam = nullptr;
    if (cameraController_.GetActiveType() == CameraType::Player)
    {
        activePlayerCam = &cameraController_.GetPlayerCamera();
    }

    // 1行でスマートに描画
    player_.Draw(activePlayerCam);

    enemyManager_.Draw();

    Model& paladinModel = ResourceManager::GetInstance().GetModelRef(ResourceKeys::Model_Paladin);
    DrawModel(paladinModel, { 0.0f, 0.0f, 3.0f }, 1.0f, WHITE);

    EndMode3D();
}

void GameContext::End()
{
    enemyManager_.Clear();
    player_.End();
    stage_.End();
}