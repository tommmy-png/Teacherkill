#include "SceneManager.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

int main()
{
    Assimp::Importer importer;
    std::cout << "Assimp ‚Ì‘g‚Ýž‚Ý‚É¬Œ÷‚µ‚Ü‚µ‚½I" << std::endl;

    SceneManager::RunConfig cfg{};
#ifdef NDEBUG
    cfg.enableDebugUI = false;
#else
    cfg.enableDebugUI = true;
#endif
    cfg.windowed = true;

    SM().SetRunConfig(cfg);
    SM().Init();
    SM().Run();
    SM().Shutdown();

    return 0;
}