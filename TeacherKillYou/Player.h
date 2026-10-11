#pragma once
#include "GameObject.h"

class Stage;
class PlayerCamera;

class Player : public GameObject
{
public:
    Player() : GameObject(GameObjectType::Player, "Player") {}
    ~Player() override = default;

    void Init();
    void Reset();
    void Update(float deltaTime, const Stage& stage,
        Vector3 forward = { 0, 0, 1 }, Vector3 right = { 1, 0, 0 });
    void Draw(const PlayerCamera* camera = nullptr) const;
    void DrawAmmoHUD() const; // Call outside BeginMode3D/EndMode3D
    void End();

    float GetRadius() const { return playerRadius_; }
    float GetHeight() const { return playerHeight_; }
    void SetColliderSize(float radius, float height)
    {
        playerRadius_ = radius;
        playerHeight_ = height;
    }

    int GetCurrentAmmo() const { return currentAmmo_; }
    int GetReserveAmmo() const { return reserveAmmo_; }



private:
    Vector3 GetInputMoveDirection(Vector3 forward, Vector3 right) const;
    void MoveWithCollision(Vector3 moveAmount, const Stage& stage);
    void UpdateVerticalPhysics(float deltaTime, const Stage& stage);

private:
    Vector3 velocity_{ 0.0f, 0.0f, 0.0f };
    float moveSpeed_{ 5.0f };
    float playerRadius_{ 0.4f };
    float playerHeight_{ 1.8f };
    float gravity_{ -9.81f };
    float jumpForce_{ 3.0f };
    bool isGrounded_{ false };
    float viewModelScale_{ 0.01f };

    Model shootModel_{};
    ModelAnimation* shootAnimations_ = nullptr;
    int shootAnimationCount_ = 0;
    bool isShooting_ = false;
    float shootTimer_ = 0.0f;
    float shootDuration_ = 0.417f;

    Model reloadModel_{};
    ModelAnimation* reloadAnimations_ = nullptr;
    int reloadAnimationCount_ = 0;
    bool isReloading_ = false;
    float reloadTimer_ = 0.0f;
    float reloadDuration_ = 2.291667f;

    int magazineCapacity_ = 15;
    int currentAmmo_ = 15;
    int reserveAmmo_ = 90;
};
