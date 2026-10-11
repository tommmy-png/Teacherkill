#include "Player.h"
#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "PlayerCamera.h"
#include "raymath.h"
#include "rlgl.h"
#include <cmath>

namespace
{
    // The GLB contains skinned hands alongside non-skinned gun meshes.
    // Updating each skinned mesh separately avoids touching missing skin data.
    void UpdateSkinnedMeshes(Model& model, const ModelAnimation& animation, float frame)
    {
        for (int i = 0; i < model.meshCount; ++i)
        {
            Mesh& mesh = model.meshes[i];
            if (mesh.boneWeights == nullptr ||
                mesh.boneIds == nullptr ||
                mesh.boneMatrices == nullptr ||
                mesh.animVertices == nullptr)
            {
                continue;
            }

            Model skinnedView = model;
            skinnedView.meshes = &model.meshes[i];
            skinnedView.meshCount = 1;
            UpdateModelAnimation(skinnedView, animation, frame);
        }
    }

    int GetAnimationFrameCount(const ModelAnimation& animation)
    {
#if defined(RAYLIB_VERSION_MAJOR) && RAYLIB_VERSION_MAJOR >= 6
        return animation.keyframeCount;
#else
        return animation.frameCount;
#endif
    }
}

void Player::Init()
{
    Reset();

    const char* shootPath = "Data/Image/HandPov/shoot_raylib.glb";
    shootModel_ = LoadModel(shootPath);
    shootAnimations_ = LoadModelAnimations(shootPath, &shootAnimationCount_);

    const char* reloadPath = "Data/Image/HandPov/reload_raylib.glb";
    reloadModel_ = LoadModel(reloadPath);
    reloadAnimations_ = LoadModelAnimations(reloadPath, &reloadAnimationCount_);
}

void Player::Reset()
{
    position_ = { 0.0f, 4.0f, 0.0f };
    rotation_ = { 0.0f, 0.0f, 0.0f };
    scale_ = { 1.0f, 1.0f, 1.0f };
    velocity_ = { 0.0f, 0.0f, 0.0f };
    isGrounded_ = false;

    isShooting_ = false;
    shootTimer_ = 0.0f;
    isReloading_ = false;
    reloadTimer_ = 0.0f;

    currentAmmo_ = magazineCapacity_;
    reserveAmmo_ = 90;
}

void Player::Update(float deltaTime, const Stage& stage, Vector3 forward, Vector3 right)
{
    Vector3 moveDir = GetInputMoveDirection(forward, right);
    if (Vector3Length(moveDir) > 0.0f)
    {
        Vector3 moveAmount = Vector3Scale(Vector3Normalize(moveDir), moveSpeed_ * deltaTime);
        MoveWithCollision(moveAmount, stage);
    }

    UpdateVerticalPhysics(deltaTime, stage);

    if (IsKeyPressed(KEY_R) && !isShooting_ && !isReloading_ &&
        currentAmmo_ < magazineCapacity_ && reserveAmmo_ > 0 &&
        reloadModel_.meshCount > 0 && reloadAnimations_ != nullptr && reloadAnimationCount_ > 0)
    {
        isReloading_ = true;
        reloadTimer_ = 0.0f;
        UpdateSkinnedMeshes(reloadModel_, reloadAnimations_[0], 0.0f);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !isShooting_ && !isReloading_ &&
        currentAmmo_ > 0 &&
        shootModel_.meshCount > 0 && shootAnimations_ != nullptr && shootAnimationCount_ > 0)
    {
        isShooting_ = true;
        shootTimer_ = 0.0f;
        --currentAmmo_;
        UpdateSkinnedMeshes(shootModel_, shootAnimations_[0], 0.0f);
    }

    if (isReloading_)
    {
        reloadTimer_ += deltaTime;
        if (reloadTimer_ >= reloadDuration_)
        {
            const int needed = magazineCapacity_ - currentAmmo_;
            const int loaded = (reserveAmmo_ < needed) ? reserveAmmo_ : needed;
            currentAmmo_ += loaded;
            reserveAmmo_ -= loaded;

            isReloading_ = false;
            reloadTimer_ = 0.0f;
        }
        else
        {
            const int frameCount = GetAnimationFrameCount(reloadAnimations_[0]);
            if (frameCount > 0)
            {
                const float progress = reloadTimer_ / reloadDuration_;
                const float frame = progress * (frameCount - 1);
                UpdateSkinnedMeshes(reloadModel_, reloadAnimations_[0], frame);
            }
        }
    }

    if (isShooting_)
    {
        shootTimer_ += deltaTime;
        if (shootTimer_ >= shootDuration_)
        {
            isShooting_ = false;
            shootTimer_ = 0.0f;
        }
        else
        {
            const int frameCount = GetAnimationFrameCount(shootAnimations_[0]);
            if (frameCount > 0)
            {
                const float progress = shootTimer_ / shootDuration_;
                const float frame = progress * (frameCount - 1);
                UpdateSkinnedMeshes(shootModel_, shootAnimations_[0], frame);
            }
        }
    }
}

Vector3 Player::GetInputMoveDirection(Vector3 forward, Vector3 right) const
{
    Vector3 moveDir = { 0.0f, 0.0f, 0.0f };
    if (IsKeyDown(KEY_W)) moveDir = Vector3Add(moveDir, forward);
    if (IsKeyDown(KEY_S)) moveDir = Vector3Subtract(moveDir, forward);
    if (IsKeyDown(KEY_A)) moveDir = Vector3Add(moveDir, right);
    if (IsKeyDown(KEY_D)) moveDir = Vector3Subtract(moveDir, right);
    return moveDir;
}

void Player::MoveWithCollision(Vector3 moveAmount, const Stage& stage)
{
    Vector3 rayPos = Vector3Add(position_, { 0.0f, 0.8f, 0.0f });

    if (moveAmount.x != 0.0f)
    {
        Vector3 dir = { moveAmount.x > 0.0f ? 1.0f : -1.0f, 0.0f, 0.0f };
        RayCollision hit = stage.Raycast({ rayPos, dir });
        if (!hit.hit || hit.distance > playerRadius_) position_.x += moveAmount.x;
    }

    if (moveAmount.z != 0.0f)
    {
        Vector3 dir = { 0.0f, 0.0f, moveAmount.z > 0.0f ? 1.0f : -1.0f };
        RayCollision hit = stage.Raycast({ rayPos, dir });
        if (!hit.hit || hit.distance > playerRadius_) position_.z += moveAmount.z;
    }
}

void Player::UpdateVerticalPhysics(float deltaTime, const Stage& stage)
{
    if (IsKeyDown(KEY_SPACE) && isGrounded_)
    {
        velocity_.y = jumpForce_;
        isGrounded_ = false;
    }

    if (!isGrounded_) velocity_.y += gravity_ * deltaTime;
    const float previousY = position_.y;
    position_.y += velocity_.y * deltaTime;
    // 移動前から落下区間を検査し、天井面の誤判定と床のすり抜けを防ぐ。
    constexpr float probeHeight = 0.25f;
    const Ray downRay = { { position_.x, previousY + probeHeight, position_.z }, { 0, -1, 0 } };
    const float probeDistance = probeHeight + (std::max)(0.0f, previousY - position_.y) + 0.01f;
    const RayCollision groundHit = stage.RaycastGround(downRay, probeDistance);
    if (velocity_.y <= 0.0f && groundHit.hit && position_.y <= groundHit.point.y + 0.01f)
    {
        position_.y = groundHit.point.y;
        velocity_.y = 0.0f;
        isGrounded_ = true;
        return;
    }
    isGrounded_ = false;
}

void Player::Draw(const PlayerCamera* camera) const
{
    Model handGun;
    if (isReloading_ && reloadModel_.meshCount > 0)
    {
        handGun = reloadModel_;
    }
    else if (isShooting_ && shootModel_.meshCount > 0)
    {
        handGun = shootModel_;
    }
    else
    {
        handGun = ResourceManager::GetInstance().GetModel(ResourceKeys::Model_HandGunView);
    }

    if (camera != nullptr)
    {
        Vector3 camPos = camera->GetPosition();
        Vector3 forward = camera->GetForwardVector();
        Vector3 right = camera->GetRightVector();
        Vector3 up = camera->GetUpVector();

        Vector3 pos = Vector3Add(camPos, Vector3Scale(forward, 0.22f));
        pos = Vector3Add(pos, Vector3Scale(right, -0.1f));
        pos = Vector3Subtract(pos, Vector3Scale(up, 0.18f));

        float yaw = atan2f(forward.x, forward.z) * RAD2DEG;

        if (isShooting_)
        {
            float progress = Clamp(shootTimer_ / shootDuration_, 0.0f, 1.0f);
            float recoil = sinf(progress * PI) * 0.045f;
            pos = Vector3Subtract(pos, Vector3Scale(forward, recoil));
            pos = Vector3Add(pos, Vector3Scale(up, recoil * 0.55f));
        }

        DrawModelEx(handGun, pos, { 0.0f, 1.0f, 0.0f }, yaw,
            { viewModelScale_, viewModelScale_, viewModelScale_ }, WHITE);
    }
    else
    {
        // System camera / debug transform
        rlPushMatrix();
        rlTranslatef(position_.x, position_.y, position_.z);
        rlRotatef(rotation_.z, 0.0f, 0.0f, 1.0f);
        rlRotatef(rotation_.x, 1.0f, 0.0f, 0.0f);
        rlRotatef(rotation_.y, 0.0f, 1.0f, 0.0f);
        rlScalef(scale_.x * viewModelScale_, scale_.y * viewModelScale_, scale_.z * viewModelScale_);

        for (int i = 0; i < handGun.meshCount; ++i)
        {
            DrawMesh(handGun.meshes[i], handGun.materials[handGun.meshMaterial[i]], MatrixIdentity());
        }
        rlPopMatrix();
    }
}

void Player::DrawAmmoHUD() const
{
    const int x = GetScreenWidth() - 210;
    const int y = GetScreenHeight() - 70;

    DrawRectangle(x - 15, y - 12, 195, 60, Fade(BLACK, 0.65f));
    DrawText(TextFormat("%02d / %02d", currentAmmo_, reserveAmmo_), x, y, 30, RAYWHITE);

    if (isReloading_)
    {
        DrawText("RELOADING", x, y - 25, 18, YELLOW);
    }
}

void Player::End()
{
    if (shootAnimations_ != nullptr)
    {
        UnloadModelAnimations(shootAnimations_, shootAnimationCount_);
        shootAnimations_ = nullptr;
        shootAnimationCount_ = 0;
    }
    if (shootModel_.meshCount > 0)
    {
        UnloadModel(shootModel_);
        shootModel_ = {};
    }

    if (reloadAnimations_ != nullptr)
    {
        UnloadModelAnimations(reloadAnimations_, reloadAnimationCount_);
        reloadAnimations_ = nullptr;
        reloadAnimationCount_ = 0;
    }
    if (reloadModel_.meshCount > 0)
    {
        UnloadModel(reloadModel_);
        reloadModel_ = {};
    }
}
