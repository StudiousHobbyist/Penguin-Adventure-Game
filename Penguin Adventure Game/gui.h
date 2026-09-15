#pragma once

#include <cmath>
#include "C:\Users\morga\Documents\VS Includes\raylib-6.0_win64_msvc16\include\raylib.h"




struct ProgressBar {
    int width;
    int height;

    float itemCount;   // current value
    float itemTarget;   // max value

    Color colorBg;
    Color colorFg;
};

inline ProgressBar MakeProgressBar(
    int width,
    int height,

    float itemCount,
    float itemTarget,

    Color colorBg = DARKGRAY,
    Color colorFg = GREEN
) {
    return ProgressBar{
        width,
        height,
        itemCount,
        itemTarget,
        colorBg,
        colorFg
    };
}

void DrawProgressBar(const ProgressBar& bar, Vector2 pos)
{
    // background
    DrawRectangle((int)pos.x, (int)pos.y, bar.width, bar.height, bar.colorBg);

    std::string text = std::to_string((int)bar.itemCount) + "/" + std::to_string((int)bar.itemTarget);



    // avoid divide-by-zero
    float t = (bar.itemTarget > 0)
        ? (float)bar.itemCount / (float)bar.itemTarget
        : 0.0f;

    if (t > 1.0f) t = 1.0f;
    if (t < 0.0f) t = 0.0f;

    // foreground fill
    DrawRectangle(
        (int)pos.x,
        (int)pos.y,
        (int)(bar.width * t),
        bar.height,
        bar.colorFg
    );

    DrawText(text.c_str(), (int)pos.x, (int)pos.y, bar.height, BLACK);
}

