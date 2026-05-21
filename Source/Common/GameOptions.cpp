#include "Common/GameOptions.h"
#include "Common/GameScreen.h"
#include "DxLib.h"
#include <cstdio>
#include <algorithm>

GameOptions g_Options;

static const char* OPTIONS_PATH = "options.dat";

void GameOptionsLoad()
{
	g_Options = {};
	FILE* fp = nullptr;
	if (fopen_s(&fp, OPTIONS_PATH, "rb") == 0 && fp)
	{
		fread(&g_Options, sizeof(GameOptions), 1, fp);
		fclose(fp);
	}
	g_Options.seVolume = std::clamp(g_Options.seVolume, 0.0f, 1.0f);
	g_Options.bgmVolume = std::clamp(g_Options.bgmVolume, 0.0f, 1.0f);
	g_Options.bulletAlpha = std::clamp(g_Options.bulletAlpha, 0.25f, 1.0f);
	if (g_Options.resWidth < 640) g_Options.resWidth = DESIGN_SCREEN_WIDTH;
	if (g_Options.resHeight < 480) g_Options.resHeight = DESIGN_SCREEN_HEIGHT;
}

void GameOptionsSave()
{
	FILE* fp = nullptr;
	if (fopen_s(&fp, OPTIONS_PATH, "wb") == 0 && fp)
	{
		fwrite(&g_Options, sizeof(GameOptions), 1, fp);
		fclose(fp);
	}
}

void GameOptionsApplyGraphics()
{
	ChangeWindowMode(g_Options.fullscreen ? FALSE : TRUE);
	SetGraphMode(g_Options.resWidth, g_Options.resHeight, 32);
	SetDrawScreen(DX_SCREEN_BACK);
	g_ScreenWidth = g_Options.resWidth;
	g_ScreenHeight = g_Options.resHeight;
	GameScreenSyncSize();
}

void GameOptionsApplyVolumes()
{
	// SE????? SoundSynth ???????? ChangeVolumeSoundMem ????f
}

int GameOptionsGetSeVolume255()
{
	return (int)(g_Options.seVolume * 255.0f);
}

int GameOptionsGetBgmVolume255()
{
	return (int)(g_Options.bgmVolume * 255.0f);
}
