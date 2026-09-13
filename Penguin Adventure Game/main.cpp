
#include <iostream>
#include "C:\Users\morga\Documents\VS Includes\raylib-6.0_win64_msvc16\include\raylib.h"



int main(void)
{
    InitWindow(800, 450, "raylib example - basic window");

    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Congrats! You created your first window!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
