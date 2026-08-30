#pragma once
#include <raylib.h>
#include "imgui.h"
#include "rlImGui.h"
#include "rollback_netcode.h"
#include "player_controller.h"

namespace FrameZero {

class EditorGUI {
public:
    bool showHitboxes = true;
    bool showPhysicsBodies = true;
    int selectedBodyId = -1;

    void initialize() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        // Polished rounded corners
        style.WindowRounding = 8.0f;
        style.FrameRounding = 6.0f;
        style.PopupRounding = 6.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 6.0f;
        style.TabRounding = 6.0f;

        // Spacing tweaks
        style.WindowPadding = ImVec2(12, 12);
        style.FramePadding = ImVec2(8, 4);
        style.ItemSpacing = ImVec2(8, 6);

        // Advanced Dark Engine Theme Colors (Unreal-style)
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.15f, 0.35f, 0.65f, 1.00f); // High-tech blue
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.25f, 0.45f, 0.85f, 0.70f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.20f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.40f, 0.70f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.35f, 0.55f, 0.95f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.45f, 0.65f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.20f, 0.35f, 0.65f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.25f, 0.45f, 0.85f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.15f, 0.30f, 0.55f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.25f, 0.45f, 0.85f, 0.80f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.30f, 0.50f, 0.90f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.20f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    }

    void draw(RollbackEngine* engine, PlayerController* players, int playerCount) {
        if (!engine) return;

        // Overlay performance metrics
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("FrameZero Debug Profiler", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("FPS: %d", GetFPS());
            ImGui::Text("Current Frame: %d", engine->getCurrentFrame());
            ImGui::Text("Confirmed Frame: %d", engine->getConfirmedFrame());
            ImGui::Text("Resimulating: %s", engine->isResimulating() ? "YES" : "NO");
            ImGui::Separator();
            ImGui::Checkbox("Show Hitboxes (Red)", &showHitboxes);
            ImGui::Checkbox("Show Physics Bodies (Blue)", &showPhysicsBodies);
        }
        ImGui::End();

        // Hierarchy and Inspector
        ImGui::SetNextWindowPos(ImVec2(GetScreenWidth() - 310.0f, 10.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Hierarchy & Inspector")) {
            ImGui::Text("Physics Bodies:");
            for (int i = 0; i < engine->bodyCount; i++) {
                PhysicsBody& b = engine->bodies[i];
                char label[64];
                sprintf(label, "Body %u (Active: %s)", b.id, b.active ? "True" : "False");
                if (ImGui::Selectable(label, selectedBodyId == (int)b.id)) {
                    selectedBodyId = (int)b.id;
                }
            }

            ImGui::Separator();
            if (selectedBodyId != -1) {
                // Find selected body
                PhysicsBody* selected = nullptr;
                for (int i = 0; i < engine->bodyCount; i++) {
                    if (engine->bodies[i].id == (uint32_t)selectedBodyId) {
                        selected = &engine->bodies[i];
                        break;
                    }
                }

                if (selected) {
                    ImGui::Text("Inspector - Body %u", selected->id);
                    
                    // Position editing
                    double px = selected->position.x.toDouble();
                    double py = selected->position.y.toDouble();
                    float pos[2] = { (float)px, (float)py };
                    if (ImGui::DragFloat2("Position", pos, 1.0f)) {
                        selected->position.x = Fixed(pos[0]);
                        selected->position.y = Fixed(pos[1]);
                    }

                    // Size editing
                    double sx = selected->size.x.toDouble();
                    double sy = selected->size.y.toDouble();
                    float size[2] = { (float)sx, (float)sy };
                    if (ImGui::DragFloat2("Size", size, 1.0f, 0.1f, 1000.0f)) {
                        selected->size.x = Fixed(size[0]);
                        selected->size.y = Fixed(size[1]);
                    }

                    // Physics properties
                    double m = selected->mass.toDouble();
                    float mass = (float)m;
                    if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 100.0f)) {
                        selected->setMass(Fixed(mass));
                    }
                    
                    double res = selected->restitution.toDouble();
                    float r = (float)res;
                    if (ImGui::SliderFloat("Restitution", &r, 0.0f, 1.0f)) {
                        selected->restitution = Fixed(r);
                    }
                }
            }
        }
        ImGui::End();

        // Frame Data / Character Tuner
        ImGui::SetNextWindowPos(ImVec2(10, 200), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Frame Data Tuning")) {
            for (int i = 0; i < playerCount; i++) {
                if (ImGui::TreeNode((void*)(intptr_t)i, "Player %d", i + 1)) {
                    PlayerController& p = players[i];
                    
                    // Movement tuning
                    float wSpeed = (float)p.walkSpeed.toDouble();
                    if (ImGui::DragFloat("Walk Speed", &wSpeed, 0.5f)) p.walkSpeed = Fixed(wSpeed);
                    
                    float jForce = (float)p.jumpForce.toDouble();
                    if (ImGui::DragFloat("Jump Force", &jForce, 5.0f)) p.jumpForce = Fixed(jForce);
                    
                    // Combat tuning
                    ImGui::Text("Attack Hitbox:");
                    float dmg = (float)p.attackHitbox.damage.toDouble();
                    if (ImGui::DragFloat("Damage", &dmg, 1.0f)) p.attackHitbox.damage = Fixed(dmg);
                    
                    ImGui::SliderInt("Hitstop (Freeze Frames)", &p.attackHitbox.hitstopDuration, 0, 30);
                    
                    float hSize[2] = { (float)p.attackHitbox.size.x.toDouble(), (float)p.attackHitbox.size.y.toDouble() };
                    if (ImGui::DragFloat2("Hitbox Size", hSize, 0.5f)) {
                        p.attackHitbox.size.x = Fixed(hSize[0]);
                        p.attackHitbox.size.y = Fixed(hSize[1]);
                    }
                    
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();
    }
};

} // namespace FrameZero
