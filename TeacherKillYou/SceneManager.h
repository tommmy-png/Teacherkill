#pragma once
#include "Scene.h"
#include "GameContext.h"
#include "TitleScene.h"
#include "GameScene.h"

enum class SceneID { Title, Game };

class SceneManager
{
public:
    static SceneManager& GetInstance()
    {
        static SceneManager instance;
        return instance;
    }

    void Init();
    void Shutdown();
    void Run();

    Scene* GetScene(SceneID id);

    GameContext& GetGameContext() { return gameContext; }

    // ÉRÉsÅ[ã÷é~
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

private:
    SceneManager() = default;
    ~SceneManager() = default;

    GameContext gameContext;
    TitleScene  titleScene{ &gameContext };
    GameScene   gameScene{ &gameContext };

    Scene* currentScene = nullptr;
};

inline SceneManager& SM() { return SceneManager::GetInstance(); }