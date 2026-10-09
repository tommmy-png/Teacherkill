#pragma once
#include <string>
#include <unordered_map>
#include "raylib.h"
#include "ResourceKeys.h"

class ResourceManager {
public:
    static ResourceManager& GetInstance();

    // キーとパスを指定してモデルをロード・登録する
    void LoadModel(const std::string& key, const std::string& path);

    // キーを指定してどこからでもモデルを取得する
    Model GetModel(const std::string& key) const;
    Model& GetModelRef(const std::string& key);

    // アニメーション読み込み用関数
    void LoadModelAnimations(const std::string& key, const std::string& path);

    // 指定したキーのアニメーションを取得（存在しない場合は count = 0）
    ModelAnimation* GetModelAnimations(const std::string& key, int* count = nullptr) const;

    // 全モデルの一括ロード／一括解放
    void LoadAll();
    void UnloadAll();

private:
    ResourceManager() = default;
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    std::unordered_map<std::string, Model> models_;

    struct AnimationData {
        ModelAnimation* anims{ nullptr };
        int count{ 0 };
    };
    std::unordered_map<std::string, AnimationData> animations_;
};

// ショートカット関数
inline ResourceManager& RM() {
    return ResourceManager::GetInstance();
}