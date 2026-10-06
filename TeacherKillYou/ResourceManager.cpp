#include "ResourceManager.h"
#include "raylib.h"
#include "rlgl.h"

#include <unordered_set>

ResourceManager& ResourceManager::GetInstance() {
    static ResourceManager instance;
    return instance;
}

// ゲーム全体で使うモデルをキーと一緒に登録しておく
void ResourceManager::LoadAll() {
    LoadModel(ResourceKeys::Model_Player, "Data/Image/greenman.glb");

	LoadModel(ResourceKeys::Model_Stage1, "Data/Image/free_loft_18_mini_office_v.optimization.glb");

    LoadModel(ResourceKeys::Model_Paladin, "Data/Image/BrainStem.glb");

    // アニメーションのロードも行う
    LoadModelAnimations(ResourceKeys::Model_Paladin, "Data/Image/BrainStem.glb");

    LoadModel(ResourceKeys::Model_Enemy, "Data/Image/monster test.glb");
    LoadModelAnimations(ResourceKeys::Model_Enemy, "Data/Image/monster test.glb");

    // 他のモデルが増えたらここに追加
    // LoadModel(ResourceKeys::Model_Stage, "Data/Image/stage.glb");
}

void ResourceManager::LoadModel(const std::string& key, const std::string& path) {
    // すでに登録済みならロードしない（二重ロード防止）
    if (models_.find(key) != models_.end()) {
        return;
    }

    Model model = ::LoadModel(path.c_str());
    models_[key] = model;
    if (model.meshCount > 0) modelBounds_[key] = GetModelBoundingBox(model);
}

// キーでモデルを取得（どのクラスからでも呼べる）
Model ResourceManager::GetModel(const std::string& key) const {
    auto it = models_.find(key);
    if (it != models_.end()) {
        return it->second;
    }
    return Model{}; // 見つからない場合は空のモデル
}

Model& ResourceManager::GetModelRef(const std::string& key)
{
    return models_[key];
}

BoundingBox ResourceManager::GetModelBounds(const std::string& key) const
{
    const auto it = modelBounds_.find(key);
    return it != modelBounds_.end() ? it->second : BoundingBox{};
}

ModelAnimation ResourceManager::GetModelAnimation(const std::string& key) const
{
    int count = 0;
    ModelAnimation* animations = GetModelAnimations(key, &count);
    if (!animations || count == 0 || animations[0].frameCount == 0) return {};
    const Model model = GetModel(key);
    return IsModelAnimationValid(model, animations[0]) ? animations[0] : ModelAnimation{};
}

void ResourceManager::LoadModelAnimations(const std::string& key, const std::string& path)
{
    if (animations_.find(key) != animations_.end()) return;

    int animCount = 0;
    ModelAnimation* anims = ::LoadModelAnimations(path.c_str(), &animCount);

    if (anims != nullptr && animCount > 0) {
        animations_[key] = { anims, animCount };
    }
}

ModelAnimation* ResourceManager::GetModelAnimations(const std::string& key, int* count) const
{
    auto it = animations_.find(key);
    if (it != animations_.end()) {
        if (count) *count = it->second.count;
        return it->second.anims;
    }
    if (count) *count = 0;
    return nullptr;
}

void ResourceManager::UnloadAll() {
    // UnloadModelは画像を解放しない。共有IDの重複解放と既定テクスチャの解放を避ける。
    std::unordered_set<unsigned int> textures;
    for (auto& pair : models_) {
        const Model& model = pair.second;
        for (int material = 0; material < model.materialCount; ++material) {
            for (int map = MATERIAL_MAP_ALBEDO; map <= MATERIAL_MAP_BRDF; ++map) {
                const Texture2D texture = model.materials[material].maps[map].texture;
                if (texture.id != 0 && texture.id != rlGetTextureIdDefault() && textures.insert(texture.id).second) {
                    ::UnloadTexture(texture);
                }
            }
        }
        ::UnloadModel(pair.second);
    }
    models_.clear();
    modelBounds_.clear();

    for (auto& pair : animations_) {
        if (pair.second.anims != nullptr) {
            ::UnloadModelAnimations(pair.second.anims, pair.second.count);
        }
    }
    animations_.clear();
}
