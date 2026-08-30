#pragma once
#include <raylib.h>
#include <cstdint>
#include "imgui.h"
#include "rlImGui.h"
#include "rollback_netcode.h"
#include "player_controller.h"

namespace FrameZero {

class EditorGUI {
public:
    bool showHitboxes = false;
    bool showPhysicsBodies = false;
    int selectedBodyId = -1;

    void initialize() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.WindowRounding = 0.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 4.0f;

        style.WindowPadding = ImVec2(8, 8);
        style.FramePadding = ImVec2(6, 4);
        style.ItemSpacing = ImVec2(8, 6);

        ImVec4* colors = style.Colors;
        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.15f, 0.35f, 0.65f, 1.00f);
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
        
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking
    }

    void draw(RollbackEngine* engine, PlayerController* players, int playerCount, RenderTexture2D* viewport) {
        if (!engine) return;

        // Create Fullscreen DockSpace
        ImGuiViewport* imguiViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(imguiViewport->WorkPos);
        ImGui::SetNextWindowSize(imguiViewport->WorkSize);
        ImGui::SetNextWindowViewport(imguiViewport->ID);
        
        ImGuiWindowFlags dockFlags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | 
                                     ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
                                     ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::Begin("DockSpace", nullptr, dockFlags);
        ImGui::PopStyleVar(3);
        
        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        // 1. Scene Viewport
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        if (ImGui::Begin("Scene Viewport")) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x > 0 && avail.y > 0) {
                // Dynamically scale the game render texture to fit the panel perfectly!
                Rectangle srcRect = { 0, 0, (float)viewport->texture.width, -(float)viewport->texture.height };
                rlImGuiImageRect(&viewport->texture, (int)avail.x, (int)avail.y, srcRect);
            }
        }
        ImGui::End();
        ImGui::PopStyleVar();

        // 2. Profiler
        if (ImGui::Begin("FrameZero Debug Profiler")) {
            ImGui::Text("FPS: %d", GetFPS());
            ImGui::Text("Current Frame: %d", engine->getCurrentFrame());
            ImGui::Text("Confirmed Frame: %d", engine->getConfirmedFrame());
            ImGui::Text("Resimulating: %s", engine->isResimulating() ? "YES" : "NO");
            ImGui::Separator();
            ImGui::Checkbox("Show Hitboxes (Red)", &showHitboxes);
            ImGui::Checkbox("Show Physics Bodies (Blue)", &showPhysicsBodies);
        }
        ImGui::End();

        // 3. Hierarchy & Inspector
        if (ImGui::Begin("Hierarchy & Inspector")) {
            ImGui::Text("Physics Bodies:");
            for (int i = 0; i < engine->bodyCount; i++) {
                PhysicsBody& b = engine->bodies[i];
                char label[64];
                // using sprintf safe alternative for gcc
                snprintf(label, sizeof(label), "Body %u (Active: %s)", b.id, b.active ? "True" : "False");
                if (ImGui::Selectable(label, selectedBodyId == (int)b.id)) {
                    selectedBodyId = (int)b.id;
                }
            }

            ImGui::Separator();
            if (selectedBodyId != -1) {
                PhysicsBody* selected = nullptr;
                for (int i = 0; i < engine->bodyCount; i++) {
                    if (engine->bodies[i].id == (uint32_t)selectedBodyId) {
                        selected = &engine->bodies[i];
                        break;
                    }
                }

                if (selected) {
                    ImGui::Text("Inspector - Body %u", selected->id);
                    
                    double px = selected->position.x.toDouble();
                    double py = selected->position.y.toDouble();
                    float pos[2] = { (float)px, (float)py };
                    if (ImGui::DragFloat2("Position", pos, 1.0f)) {
                        selected->position.x = Fixed(pos[0]);
                        selected->position.y = Fixed(pos[1]);
                    }

                    double sx = selected->size.x.toDouble();
                    double sy = selected->size.y.toDouble();
                    float size[2] = { (float)sx, (float)sy };
                    if (ImGui::DragFloat2("Size", size, 1.0f, 0.1f, 1000.0f)) {
                        selected->size.x = Fixed(size[0]);
                        selected->size.y = Fixed(size[1]);
                    }

                    double m = selected->mass.toDouble();
                    float mass = (float)m;
                    if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.0f, 100.0f)) {
                        selected->setMass(Fixed(mass));
                    }
                }
            }
        }
        ImGui::End();

        // 4. Frame Data Tuning
        if (ImGui::Begin("Frame Data Tuning")) {
            for (int i = 0; i < playerCount; i++) {
                if (ImGui::TreeNode((void*)(intptr_t)i, "Player %d", i + 1)) {
                    PlayerController& p = players[i];
                    
                    float wSpeed = (float)p.walkSpeed.toDouble();
                    if (ImGui::DragFloat("Walk Speed", &wSpeed, 0.5f)) p.walkSpeed = Fixed(wSpeed);
                    
                    float jForce = (float)p.jumpForce.toDouble();
                    if (ImGui::DragFloat("Jump Force", &jForce, 5.0f)) p.jumpForce = Fixed(jForce);
                    
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
        
        ImGui::End(); // End DockSpace
    }
};

} // namespace FrameZero
