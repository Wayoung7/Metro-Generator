#include "metro.hpp"
#include "raygui.h"
#include <algorithm>
#include <raylib.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

constexpr static const int windowWidth = 1600;
constexpr static const int windowHeight = 1000;

static Vector2 window_position = {20, 20};
static Vector2 window_size = {300, 600};
static bool minimized = false;
static bool moving = false;
static bool resizing = false;
static Vector2 scroll;

void GuiWindowFloating(Vector2* position, Vector2* size, bool* minimized,
                       bool* moving, bool* resizing,
                       void (*draw_content)(Vector2, Vector2),
                       Vector2 content_size, Vector2* scroll,
                       const char* title) {
#if !defined(RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT)
#define RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT 38
#endif

#if !defined(RAYGUI_WINDOW_CLOSEBUTTON_SIZE)
#define RAYGUI_WINDOW_CLOSEBUTTON_SIZE 38
#endif

    int close_title_size_delta_half =
        (RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT - RAYGUI_WINDOW_CLOSEBUTTON_SIZE) /
        2;

    // window movement and resize input and collision check
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !*moving && !*resizing) {
        Vector2 mouse_position = GetMousePosition();

        Rectangle title_collision_rect = {
            position->x, position->y,
            size->x -
                (RAYGUI_WINDOW_CLOSEBUTTON_SIZE + close_title_size_delta_half),
            RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT};
        Rectangle resize_collision_rect = {position->x + size->x - 20,
                                           position->y + size->y - 20, 20, 20};

        if (CheckCollisionPointRec(mouse_position, title_collision_rect)) {
            *moving = true;
        } else if (!*minimized && CheckCollisionPointRec(
                                      mouse_position, resize_collision_rect)) {
            *resizing = true;
        }
    }

    // window movement and resize update
    if (*moving) {
        Vector2 mouse_delta = GetMouseDelta();
        position->x += mouse_delta.x;
        position->y += mouse_delta.y;

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *moving = false;

            // clamp window position keep it inside the application area
            if (position->x < 0)
                position->x = 0;
            else if (position->x > GetScreenWidth() - size->x)
                position->x = GetScreenWidth() - size->x;
            if (position->y < 0)
                position->y = 0;
            else if (position->y > GetScreenHeight())
                position->y =
                    GetScreenHeight() - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT;
        }

    } else if (*resizing) {
        Vector2 mouse = GetMousePosition();
        if (mouse.x > position->x)
            size->x = mouse.x - position->x;
        if (mouse.y > position->y)
            size->y = mouse.y - position->y;

        // clamp window size to an arbitrary minimum value and the window size
        // as the maximum
        if (size->x < 100)
            size->x = 100;
        else if (size->x > GetScreenWidth())
            size->x = GetScreenWidth();
        if (size->y < 100)
            size->y = 100;
        else if (size->y > GetScreenHeight())
            size->y = GetScreenHeight();

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            *resizing = false;
        }
    }

    // window and content drawing with scissor and scroll area
    if (*minimized) {
        GuiStatusBar((Rectangle){position->x, position->y, size->x,
                                 RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT},
                     title);

        if (GuiButton((Rectangle){position->x + size->x -
                                      RAYGUI_WINDOW_CLOSEBUTTON_SIZE -
                                      close_title_size_delta_half,
                                  position->y + close_title_size_delta_half,
                                  RAYGUI_WINDOW_CLOSEBUTTON_SIZE,
                                  RAYGUI_WINDOW_CLOSEBUTTON_SIZE},
                      "#120#")) {
            *minimized = false;
        }

    } else {
        *minimized = GuiWindowBox(
            (Rectangle){position->x, position->y, size->x, size->y}, title);

        // scissor and draw content within a scroll panel
        if (draw_content != NULL) {
            Rectangle scissor = {0};
            GuiScrollPanel(
                (Rectangle){position->x,
                            position->y + RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT,
                            size->x,
                            size->y - RAYGUI_WINDOWBOX_STATUSBAR_HEIGHT},
                NULL,
                (Rectangle){position->x, position->y, content_size.x,
                            content_size.y},
                scroll, &scissor);

            bool require_scissor =
                size->x < content_size.x || size->y < content_size.y;

            if (require_scissor) {
                BeginScissorMode(scissor.x, scissor.y, scissor.width,
                                 scissor.height);
            }

            draw_content(*position, *scroll);

            if (require_scissor) {
                EndScissorMode();
            }
        }

        // draw the resize button/icon
        GuiDrawIcon(71, position->x + size->x - 20, position->y + size->y - 20,
                    1, WHITE);
    }
}

static void DrawContent(Vector2 position, Vector2 scroll) {
    GuiButton((Rectangle){position.x + 20 + scroll.x,
                          position.y + 65 + scroll.y, 260, 60},
              "Generate");
}

int main() {
    auto cfg = Config{
        .mapWidth = 1000,
        .mapHeight = 1000,
    };

    // int mapWidth = 5000;
    // int mapHeight = 5000;
    // int textureWidth = 1000;
    float scaleFactor = std::min(
        static_cast<float>(windowWidth) / static_cast<float>(cfg.mapWidth),
        static_cast<float>(windowHeight) / static_cast<float>(cfg.mapHeight));

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(1600, 1000, "Metro Generator - by Wayoung7");
    SetTargetFPS(60);
    Camera2D cam{Vector2{windowWidth / 2.f, windowHeight / 2.f}, Vector2Zero(),
                 0.f, scaleFactor};
    Metro metro(cfg);
    metro.build();
    RenderTexture2D metroTex = LoadRenderTexture(windowWidth, windowHeight);
    GuiLoadStyle("candy.rgs");
    // guiFont.baseSize = 5;
    while (!WindowShouldClose()) {

        BeginTextureMode(metroTex);
        ClearBackground(RAYWHITE);
        BeginMode2D(cam);
        metro.draw(cam);
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        // Render gui

        DrawTextureRec(metroTex.texture,
                       Rectangle{0.f, 0.f, static_cast<float>(windowWidth),
                                 static_cast<float>(windowHeight)},
                       Vector2{0.f, 0.f}, WHITE);
        GuiWindowFloating(&window_position, &window_size, &minimized, &moving,
                          &resizing, &DrawContent, (Vector2){140, 320}, &scroll,
                          "Settings");
        EndDrawing();
    }
    UnloadRenderTexture(metroTex);
    CloseWindow();
    return 0;
}

RenderTexture2D genRenderTexture(const Metro& m, const Camera2D& cam) {
    RenderTexture2D metroTex = LoadRenderTexture(windowWidth, windowHeight);
    BeginTextureMode(metroTex);
    ClearBackground(RAYWHITE);
    BeginMode2D(cam);
    m.draw(cam);
    EndTextureMode();
    return metroTex;
}