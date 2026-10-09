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
    // 現在のアクティブカメラが PlayerCamera かどうかチェック
    bool isPlayerCameraActive = (cameraController_.GetActiveType() == CameraType::Player);

    // 1. プレイヤーカメラがアクティブな時のみ、プレイヤーの移動操作を行う
    if (isPlayerCameraActive)
    {
        Vector3 forward = cameraController_.GetPlayerCamera().GetForwardVector();
        Vector3 right = cameraController_.GetPlayerCamera().GetRightVector();

        // プレイヤーの更新
        player_.Update(deltaTime, stage_, forward, right);
    }

    // 2. ステージや敵の更新（システムカメラ中もゲームワールド自体の時間は動かす）
    stage_.Update(deltaTime);
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

    // 3. プレイヤーカメラアクティブ時のみ追従処理を行う
    if (isPlayerCameraActive)
    {
        cameraController_.GetPlayerCamera().SetPlayerPosition(player_.GetPosition());
    }

    // 4. カメラコントローラー全体の更新（K/Lキーの監視やアクティブカメラの更新を行う）
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
    DrawModel(paladinModel, { 0.0f, 0.0f, 3.0f }, 1.0f, WHITE);

    EndMode3D();
}

void GameContext::End()
{
    enemyManager_.Clear();
    player_.End();
    stage_.End();
}