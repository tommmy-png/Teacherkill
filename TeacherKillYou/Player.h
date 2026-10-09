#pragma once
#include "raylib.h"

class Stage;

class Player
{
public:
    Player() = default;
    ~Player() = default;

    void Init();
    void Reset();
    void Update(float deltaTime, const Stage& stage, Vector3 forward = { 0,0,1 }, Vector3 right = { 1,0,0 });
    void Draw() const;
    void End();

    // Transform
    Vector3 GetPosition() const { return position_; }
    void SetPosition(Vector3 pos) { position_ = pos; }

    Vector3 GetRotation() const { return rotation_; }
    void SetRotation(Vector3 rot) { rotation_ = rot; }

    Vector3 GetScale() const { return scale_; }
    void SetScale(Vector3 scl) { scale_ = scl; }

    // 当たり判定パラメータ用
    float GetRadius() const { return playerRadius_; }
    float GetHeight() const { return playerHeight_; }
    void SetColliderSize(float radius, float height) { playerRadius_ = radius; playerHeight_ = height; }

private:
    Vector3 position_{ 0.0f, 0.0f, 0.0f };
    Vector3 rotation_{ 0.0f, 0.0f, 0.0f }; // 回転 (Yaw, Pitch, Roll)
    Vector3 scale_{ 1.0f, 1.0f, 1.0f };    // モデル拡大率

    Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    float moveSpeed_{ 5.0f };

    float playerRadius_{ 0.4f };
    float playerHeight_{ 1.8f };

    float gravity_{ -9.81f };
    float jumpForce_{ 3.0f };

    bool isGrounded_{ false };

};