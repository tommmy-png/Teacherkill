#include "Stage.h"
#include "ResourceManager.h"
#include "ResourceKeys.h"
#include "raymath.h"

#include <algorithm>
#include <cmath>

namespace {

// Conservative slab test, including rays starting inside a box and flat meshes.
bool RayReachesBounds(Ray ray, BoundingBox bounds, float maxDistance) {
    constexpr float boundsPadding = 0.0001f;
    const float origins[] = { ray.position.x, ray.position.y, ray.position.z };
    const float directions[] = { ray.direction.x, ray.direction.y, ray.direction.z };
    const float minimums[] = { bounds.min.x, bounds.min.y, bounds.min.z };
    const float maximums[] = { bounds.max.x, bounds.max.y, bounds.max.z };
    float nearDistance = 0.0f;
    float farDistance = maxDistance;
    for (int axis = 0; axis < 3; ++axis) {
        const float minimum = minimums[axis] - boundsPadding;
        const float maximum = maximums[axis] + boundsPadding;
        if (directions[axis] == 0.0f) {
            if (origins[axis] < minimum || origins[axis] > maximum) return false;
            continue;
        }
        const float a = (minimum - origins[axis]) / directions[axis];
        const float b = (maximum - origins[axis]) / directions[axis];
        nearDistance = (std::max)(nearDistance, (std::min)(a, b));
        farDistance = (std::min)(farDistance, (std::max)(a, b));
        if (nearDistance > farDistance) return false;
    }
    return true;
}

} // namespace

void Stage::Init()
{
    Reset();
    CacheCollisionBounds(RM().GetModel(ResourceKeys::Model_Stage1));
}

void Stage::Reset()
{
    position_ = { 0.0f, 0.0f, 0.0f };
    scale_ = 1.0f;
    cachedMeshes_ = nullptr;
    meshBounds_.clear();
}

void Stage::Update(float deltaTime)
{
}

void Stage::Draw() const
{
    Model stageModel = ResourceManager::GetInstance().GetModel(ResourceKeys::Model_Stage1);
    DrawModel(stageModel, position_, scale_, WHITE);
}

void Stage::End()
{
    cachedMeshes_ = nullptr;
    meshBounds_.clear();
}

void Stage::CacheCollisionBounds(const Model& model) const
{
    if (cachedMeshes_ == model.meshes && meshBounds_.size() == static_cast<std::size_t>(model.meshCount)) return;
    meshBounds_.clear();
    if (model.meshes && model.meshCount > 0) {
        meshBounds_.reserve(model.meshCount);
        for (int i = 0; i < model.meshCount; ++i) {
            meshBounds_.push_back(GetMeshBoundingBox(model.meshes[i]));
        }
    }
    cachedMeshes_ = model.meshes;
}

RayCollision Stage::Raycast(Ray ray) const
{
    return Raycast(ray, 999999.0f);
}

RayCollision Stage::Raycast(Ray ray, float maxDistance) const
{
    RayCollision closestHit = {};
    closestHit.distance = maxDistance;
    if (std::isnan(maxDistance) || maxDistance < 0.0f) return closestHit;
    const Model stageModel = RM().GetModel(ResourceKeys::Model_Stage1);
    if (!stageModel.meshes || stageModel.meshCount <= 0) return closestHit;
    CacheCollisionBounds(stageModel);

    // Bounds are cached in model space; moving the stage needs no rebuild.
    const Ray localRay = { Vector3Subtract(ray.position, position_), ray.direction };
    const Matrix transform = MatrixTranslate(position_.x, position_.y, position_.z);
    for (int i = 0; i < stageModel.meshCount; ++i) {
        if (!RayReachesBounds(localRay, meshBounds_[i], closestHit.distance)) continue;
        const RayCollision hit = GetRayCollisionMesh(ray, stageModel.meshes[i], transform);
        if (hit.hit && hit.distance <= maxDistance && (!closestHit.hit || hit.distance < closestHit.distance)) {
            closestHit = hit;
        }
    }
    return closestHit;
}
