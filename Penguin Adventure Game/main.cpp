// main.cpp

#include "C:\Users\morga\Documents\VS Includes\raylib-6.0_win64_msvc16\include\raylib.h"
#include <string>
#include <map>
#include <array>

#include "gui.h"
#include "world.h"
#include "entity.h"

//will be changed once settings menu is added
int TARGETFPS = 200;

float WINDOWWIDTH = 800.0f;
float WINDOWHEIGHT = 450.0f;

float CAMERASPEED = 500.0f;

float CAMERAZOOM = 1.0f;

enum class CameraModeT {
    Free,
    Player
};


void input(Camera2D& camera, EntityManager& em, float dt) {
    Entity* e = em.CurrentEntity();

    Vector2 move{ 0,0 };

    if (IsKeyDown(KEY_A)) {
        move.x = -1;
    }
    if (IsKeyDown(KEY_D)) {
        move.x = 1;
    }
    if (IsKeyDown(KEY_W)) {
        move.y = -1;
    }
    if (IsKeyDown(KEY_S)) {
        move.y = 1;
    }

    if (move.x != 0 || move.y != 0)
    {
        move = Normalize(move);
    }

    if (e) {
        e->Move(move.x * 10, move.y * 10, dt);
        camera.target.x += (e->GetPos().x - camera.target.x) / (10 / camera.zoom);
        camera.target.y += (e->GetPos().y - camera.target.y) / (10 / camera.zoom);
    }


    CAMERAZOOM += GetMouseWheelMove() * 2.0f * dt;

    if (CAMERAZOOM < 0.05f)
        CAMERAZOOM = 0.05f;

    if (CAMERAZOOM > 10.0f)
        CAMERAZOOM = 10.0f;

    camera.zoom += (CAMERAZOOM - camera.zoom) / 10;
}

int main()
{

    InitWindow((int)WINDOWWIDTH, (int)WINDOWHEIGHT, "raylib example - basic window");

    SetTargetFPS(TARGETFPS);

    ProgressBar bar = MakeProgressBar(500, 50, 0, 1000);

    float uiBuffer = 10;

    EntityManager em;
    ChunkManager cm;

    em.AddEntity();

    Camera2D camera = { 0 };

    camera.target = { 0, 0 };
    camera.offset = { WINDOWWIDTH / 2, WINDOWHEIGHT / 2 };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // ToggleFullscreen();
    WINDOWWIDTH = GetScreenWidth();
    WINDOWHEIGHT = GetScreenHeight();
    camera.offset = { WINDOWWIDTH / 2, WINDOWHEIGHT / 2 };

    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        // input

        input(camera, em, dt);

        cm.UpdateChunks(camera, 500.0);
        cm.ProcessCompletedChunks(0.1);
        cm.clean(camera, 5.0);

        BeginDrawing();

        ClearBackground(RAYWHITE);

        // render
        BeginMode2D(camera);

        cm.DrawChunks(camera);
        em.DrawEntities(cm, dt);

        EndMode2D();

        // gui
        DrawFPS(200, 0);

        cm.DrawDiagnostics();
        // DrawProgressBar(bar, { uiBuffer, uiBuffer });

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
