#pragma once

#include "GameConfig.h"

struct GameData
{
	Difficulty difficulty = Difficulty::Normal;
	int score = 0;
	int blocksBroken = 0;
	int livesLeft = START_LIVES;
	int maxCombo = 0;
	int playTimeSec = 0;
	bool isClear = false;
	bool isStalemate = false;
	bool isGameOver = false;
};

extern GameData g_GameData;
