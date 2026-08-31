---
name: imgui-viewport-scaling
description: Rule for scaling embedded framebuffers correctly within ImGui DockSpaces.
---

# ImGui Viewport Scaling Rule

## Rationale
When embedding a game engine or application viewport into an ImGui UI (such as Raylib `RenderTexture2D` or OpenGL framebuffers), rendering the texture directly at its native resolution causes "broken fullscreen" behavior where the image clips or fails to fill the dockable window. 

To ensure the game viewport correctly conforms to the ImGui layout and resizes dynamically, you must always query the available content region of the ImGui window before drawing the texture.

## The Rule
Whenever you render a framebuffer/texture inside an ImGui window (e.g. `rlImGuiImageRenderTexture`), you **MUST** scale it to fit the window dynamically.

**Incorrect Pattern:**
```cpp
if (ImGui::Begin("Scene Viewport")) {
    rlImGuiImageRenderTexture(viewport);
}
ImGui::End();
```

**Correct Pattern:**
```cpp
if (ImGui::Begin("Scene Viewport")) {
    ImVec2 avail = ImGui::GetContentRegionAvail();
    if (avail.x > 0 && avail.y > 0) {
        Rectangle srcRect = { 0, 0, (float)viewport->texture.width, -(float)viewport->texture.height };
        rlImGuiImageRect(&viewport->texture, (int)avail.x, (int)avail.y, srcRect);
    }
}
ImGui::End();
```

## Application
This rule will be applied whenever the user requests an embedded Editor GUI, a game Viewport, or mentions "fullscreen broken" when dealing with ImGui framebuffers.
