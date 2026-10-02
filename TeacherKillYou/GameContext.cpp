#include "GameContext.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "raylib.h"

void GameContext::Init()
{
    // カメラコントローラーの初期化（ウィンドウ生成後に呼ばれ、PlayerCameraとカーソル非表示を確定させる）
    cameraController_.Init();
	player_.Init();
}

void GameContext::Reset()
{
	player_.Reset();
}

void GameContext::Update(float deltaTime)
{
    // カメラの向きに合わせてプレイヤーを動かす
    Vector3 forward = cameraController_.GetPlayerCamera().GetForwardVector();
    Vector3 right = cameraController_.GetPlayerCamera().GetRightVector();
    player_.Update(deltaTime, forward, right);

    // プレイヤーの移動後位置をカメラに教える
    cameraController_.GetPlayerCamera().SetPlayerPosition(player_.GetPosition());

    // カメラ全体のUpdateを呼ぶ
    cameraController_.Update();
}

void GameContext::Draw() const
{
    BeginMode3D(cameraController_.GetActiveRaylibCamera());

    DrawGrid(20, 1.0f);

    player_.Draw();

    EndMode3D();
}

void GameContext::End()
{
    player_.End();
}