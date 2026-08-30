#include <raylib.h>
#include <iostream>

int main() {
    InitWindow(800, 600, "Test");
    Texture2D tex = LoadTexture("character.jpg");
    std::cout << "Texture ID: " << tex.id << " Size: " << tex.width << "x" << tex.height << std::endl;
    CloseWindow();
    return 0;
}
