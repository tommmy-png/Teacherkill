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
    // 1. カメラの向きを取得
    Vector3 forward = cameraController_.GetPlayerCamera().GetForwardVector();
    Vector3 right = cameraController_.GetPlayerCamera().GetRightVector();

    // 2. プレイヤーの移動（ステージとの当たり判定を含めて更新）
    player_.Update(deltaTime, stage_, forward, right);

    // 3. ステージの更新
    stage_.Update(deltaTime);
    enemyManager_.Update(deltaTime, player_, stage_);
    enemyManager_.RemoveExpired();

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

    EndMode3D();
}

void GameContext::End()
{
    enemyManager_.Clear();
    player_.End();
    stage_.End();
}