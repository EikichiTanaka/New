#pragma once

constexpr int DESIGN_SCREEN_WIDTH = 1280;
constexpr int DESIGN_SCREEN_HEIGHT = 720;

extern int g_ScreenWidth;
extern int g_ScreenHeight;

#define SCREEN_WIDTH   (g_ScreenWidth)
#define SCREEN_HEIGHT  (g_ScreenHeight)

void GameScreenSyncSize();
