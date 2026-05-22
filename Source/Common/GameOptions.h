#pragma once

struct GameOptions
{
	float seVolume = 1.0f;
	float bgmVolume = 0.7f;
	bool  fullscreen = false;
	int   resWidth = 1280;
	int   resHeight = 720;
	float bulletAlpha = 1.0f;
	bool  hitStopEnabled = true;
};

extern GameOptions g_Options;

void GameOptionsLoad();
void GameOptionsSave();
void GameOptionsApplyGraphics();
void GameOptionsApplyVolumes();

int GameOptionsGetSeVolume255();
int GameOptionsGetBgmVolume255();
