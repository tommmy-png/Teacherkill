#pragma once
#include "raylib.h"
#include <vector>

class Stage
{
public:
    Stage() = default;
    ~Stage() = default;

    void Init();
    void Reset();
    void Update(float deltaTime);
    void Draw() const;
    void End();

    // レイキャストを実行し、衝突情報（ヒットした座標や距離）を返す
    RayCollision Raycast(Ray ray) const;
    // Limit collision work to the portion of the ray needed by the caller.
    RayCollision Raycast(Ray ray, float maxDistance) const;

    // 当たり判定（プレイヤーのバウンディングボックスとの衝突チェック）
    bool CheckCollision(BoundingBox playerBox) const;

    Vector3 GetPosition() const { return position_; }
    void SetPosition(Vector3 pos) { position_ = pos; }

    float GetScale() const { return scale_; }
    void SetScale(float scale) { scale_ = scale; }

private:
    void CacheCollisionBounds(const Model& model) const;
    mutable const Mesh* cachedMeshes_ = nullptr;
    mutable std::vector<BoundingBox> meshBounds_;

    Vector3 position_{ 0.0f, 0.0f, 0.0f };
    float scale_{ 1.0f };
};