#pragma once
#include "CameraController.h"
#include "Player.h"
#include "Stage.h"
#include "EnemyManager.h"

class GameContext
{
public:
    GameContext() = default;
    ~GameContext() = default;

    void Init();
    void Reset();
    void Update(float deltaTime);
    void Draw() const;
    void End();

    CameraController& GetCameraController() { return cameraController_; }
    const CameraController& GetCameraController() const { return cameraController_; }

    Player& GetPlayer() { return player_; }
    const Player& GetPlayer() const { return player_; }

    Stage& GetStage() { return stage_; }
    const Stage& GetStage() const { return stage_; }

    demo::EnemyManager& GetEnemyManager() { return enemyManager_; }
    const demo::EnemyManager& GetEnemyManager() const { return enemyManager_; }

private:
    CameraController cameraController_;
    Player player_;
    Stage stage_;
    demo::EnemyManager enemyManager_;

    int animFrame_{ 0 };
    int animIndex_{ 0 };
};
