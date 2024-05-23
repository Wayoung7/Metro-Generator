#include "metro.hpp"
#include "ui.hpp"
#include <algorithm>
#include <raylib.h>

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

constexpr static const int windowWidth = 1200;
constexpr static const int windowHeight = 800;

Config cfg;
UI ui;

float scaleFactor = std::min(
    static_cast<float>(windowWidth) / static_cast<float>(cfg.mapWidth),
    static_cast<float>(windowHeight) / static_cast<float>(cfg.mapHeight));

RenderTexture2D metroTex;
Camera2D cam{Vector2{windowWidth / 2.f, windowHeight / 2.f}, Vector2Zero(), 0.f,
             scaleFactor};
Metro metro(cfg);

static void GenerateButton(const Config& cfg, Metro& m) {
    m.clearAndReconfigure(cfg);
    m.build();
}

RenderTexture2D genRenderTexture(const Metro& m, const Camera2D& cam) {
    RenderTexture2D metroTex = LoadRenderTexture(windowWidth, windowHeight);
    BeginTextureMode(metroTex);
    ClearBackground(Color{242, 230, 206, 255});
    BeginMode2D(cam);
    m.draw(cam);
    EndTextureMode();
    return metroTex;
}

void UpdateDrawFrame() {
    BeginDrawing();

    DrawTextureRec(metroTex.texture,
                   Rectangle{0.f, 0.f, static_cast<float>(windowWidth),
                             static_cast<float>(windowHeight)},
                   Vector2{0.f, 0.f}, WHITE);
    auto action = ui.handleUI(cfg);
    if (action == GenerateAction) {
        GenerateButton(cfg, metro);
        scaleFactor = std::min(static_cast<float>(windowWidth) /
                                   static_cast<float>(cfg.mapWidth),
                               static_cast<float>(windowHeight) /
                                   static_cast<float>(cfg.mapHeight));
        cam.zoom = scaleFactor;
        UnloadRenderTexture(metroTex);
        metroTex = genRenderTexture(metro, cam);
    } else if (action == ExportImageAction) {
        ExportImage(LoadImageFromTexture(metroTex.texture), "metro.png");
    }
    EndDrawing();
}

int main() {

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(windowWidth, windowHeight, "Metro Generator - by Wayoung7");
    GuiLoadStyle("light.rgs");
    metro.build();
    metroTex = genRenderTexture(metro, cam);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    UnloadRenderTexture(metroTex);
    CloseWindow();
    return 0;
}

