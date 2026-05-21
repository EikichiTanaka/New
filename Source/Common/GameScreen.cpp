#include "Common/GameScreen.h"
#include "DxLib.h"

int g_ScreenWidth = DESIGN_SCREEN_WIDTH;
int g_ScreenHeight = DESIGN_SCREEN_HEIGHT;

void GameScreenSyncSize()
{
	int w = 0, h = 0;
	if (GetDrawScreenSize(&w, &h) == 0 && w > 0 && h > 0)
	{
		g_ScreenWidth = w;
		g_ScreenHeight = h;
	}
}
