#include "DebugUI.h"
#include "GameContext.h"
#include "imgui.h"
#include "ImGuizmo.h"
#include "raymath.h"

void DebugUI::Draw(GameContext& gameContext)
{
    auto& cameraController = gameContext.GetCameraController();

    if (cameraController.GetActiveType() == CameraType::System)
    {
        ImGuizmo::BeginFrame();

        DrawHierarchy();
        DrawInspector(gameContext);
        DrawGizmo(gameContext);
    }

    DrawCameraController(cameraController);
}

void DebugUI::DrawGizmo(GameContext& gameContext)
{
    if (selectedObject_ != SelectedObjectType::Player)
    {
        return;
    }

    if (!ImGui::GetIO().WantCaptureKeyboard)
    {
        if (IsKeyPressed(KEY_W)) currentGizmoOperation_ = ImGuizmo::TRANSLATE;
        if (IsKeyPressed(KEY_E)) currentGizmoOperation_ = ImGuizmo::ROTATE;
        if (IsKeyPressed(KEY_R)) currentGizmoOperation_ = ImGuizmo::SCALE;
    }

    auto& cameraController = gameContext.GetCameraController();
    const Camera3D& camera = cameraController.GetActiveRaylibCamera();
    Player& player = gameContext.GetPlayer();

    ImGuiIO& io = ImGui::GetIO();

    ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());
    ImGuizmo::SetRect(0.0f, 0.0f, io.DisplaySize.x, io.DisplaySize.y);
    ImGuizmo::Enable(true);

    float aspect = io.DisplaySize.x / io.DisplaySize.y;

    Matrix rlView = GetCameraMatrix(camera);
    Matrix rlProj = MatrixPerspective(camera.fovy * DEG2RAD, aspect, 0.01f, 1000.0f);

    Matrix viewMat = MatrixTranspose(rlView);
    Matrix projMat = MatrixTranspose(rlProj);

    Vector3 pos = player.GetPosition();
    Vector3 rot = player.GetRotation();
    Vector3 scale = player.GetScale();

    // ★ 回転行列の順序を明確に計算 (Y * X * Z)
    Matrix rlScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix rlRotX = MatrixRotateX(rot.x * DEG2RAD);
    Matrix rlRotY = MatrixRotateY(rot.y * DEG2RAD);
    Matrix rlRotZ = MatrixRotateZ(rot.z * DEG2RAD);

    // Z * X * Y の順で回転を合成
    Matrix rlRot = MatrixMultiply(MatrixMultiply(rlRotZ, rlRotX), rlRotY);
    Matrix rlTrans = MatrixTranslate(pos.x, pos.y, pos.z);

    Matrix rlModel = MatrixMultiply(MatrixMultiply(rlScale, rlRot), rlTrans);
    Matrix modelMat = MatrixTranspose(rlModel);

    bool manipulated = ImGuizmo::Manipulate(
        &viewMat.m0,
        &projMat.m0,
        currentGizmoOperation_,
        ImGuizmo::WORLD,
        &modelMat.m0
    );

    if (manipulated)
    {
        float matrix[16];
        memcpy(matrix, &modelMat.m0, sizeof(float) * 16);

        float matrixTranslation[3];
        float matrixRotation[3];
        float matrixScale[3];

        ImGuizmo::DecomposeMatrixToComponents(
            matrix,
            matrixTranslation,
            matrixRotation,
            matrixScale
        );

        player.SetPosition({ matrixTranslation[0], matrixTranslation[1], matrixTranslation[2] });
        player.SetRotation({ matrixRotation[0], matrixRotation[1], matrixRotation[2] });
        player.SetScale({ matrixScale[0], matrixScale[1], matrixScale[2] });
    }
}

void DebugUI::DrawHierarchy()
{
    ImGui::Begin("Hierarchy");

    if (ImGui::Selectable("Player", selectedObject_ == SelectedObjectType::Player))
    {
        selectedObject_ = SelectedObjectType::Player;
    }

    if (ImGui::Selectable("Enemy Manager", selectedObject_ == SelectedObjectType::EnemyManager))
    {
        selectedObject_ = SelectedObjectType::EnemyManager;
    }

    if (ImGui::Selectable("Stage", selectedObject_ == SelectedObjectType::Stage))
    {
        selectedObject_ = SelectedObjectType::Stage;
    }

    if (ImGui::Selectable("Paladin Model", selectedObject_ == SelectedObjectType::PaladinModel))
    {
        selectedObject_ = SelectedObjectType::PaladinModel;
    }

    ImGui::End();
}

void DebugUI::DrawInspector(GameContext& gameContext)
{
    ImGui::Begin("Inspector");

    switch (selectedObject_)
    {
    case SelectedObjectType::Player:
    {
        Player& player = gameContext.GetPlayer();

        Vector3 position = player.GetPosition();
        Vector3 rotation = player.GetRotation();
        Vector3 scale = player.GetScale();

        ImGui::Text("Player Transform");

        if (ImGui::DragFloat3("Position", &position.x, 0.1f))
        {
            player.SetPosition(position);
        }

        if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
        {
            player.SetRotation(rotation);
        }

        if (ImGui::DragFloat3("Scale", &scale.x, 0.05f, 0.1f, 10.0f))
        {
            player.SetScale(scale);
        }

        break;
    }

    case SelectedObjectType::EnemyManager:
        ImGui::Text("Enemy Manager");
        break;

    case SelectedObjectType::Stage:
        ImGui::Text("Stage");
        break;

    case SelectedObjectType::PaladinModel:
        ImGui::Text("Paladin Model");
        break;

    default:
        ImGui::Text("No object selected.");
        break;
    }

    ImGui::End();
}

void DebugUI::DrawCameraController(CameraController& cameraController)
{
    ImGui::Begin("Camera Controller");

    const bool isPlayerCamera =
        cameraController.GetActiveType() == CameraType::Player;

    if (ImGui::RadioButton("Player Camera", isPlayerCamera))
    {
        cameraController.SetActiveCamera(CameraType::Player);
    }

    ImGui::SameLine();

    if (ImGui::RadioButton("System Camera", !isPlayerCamera))
    {
        cameraController.SetActiveCamera(CameraType::System);
    }

    const auto& camera = cameraController.GetActiveRaylibCamera();

    ImGui::Separator();

    ImGui::Text("Pos   : (%.2f, %.2f, %.2f)",
        camera.position.x,
        camera.position.y,
        camera.position.z);

    ImGui::Text("Target: (%.2f, %.2f, %.2f)",
        camera.target.x,
        camera.target.y,
        camera.target.z);

    ImGui::End();
}