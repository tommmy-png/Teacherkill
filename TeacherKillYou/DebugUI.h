#pragma once

#include "CameraController.h"
#include "imgui.h"
#include "ImGuizmo.h"

enum class SelectedObjectType
{
    None,
    Player,
    EnemyManager,
    Stage,
    PaladinModel
};

class GameContext;

class DebugUI
{
public:
    void Draw(GameContext& gameContext);

private:
    void DrawHierarchy();
    void DrawInspector(GameContext& gameContext);
    void DrawGizmo(GameContext& gameContext);
    void DrawCameraController(CameraController& cameraController);

    SelectedObjectType selectedObject_{ SelectedObjectType::None };

    ImGuizmo::OPERATION currentGizmoOperation_{ ImGuizmo::TRANSLATE };
};