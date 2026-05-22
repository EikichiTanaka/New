#pragma once
#include "GameConfig.h"

struct RunStats
{
	int maxKillChain = 0;
	int feverActivations = 0;
	int bombsUsed = 0;
	int enemiesDestroyed = 0;
	int bossMaxPhaseReached = 0;
	bool clearedWithoutDamage = true;
	bool noBombUsed = true;
	int  wavesNoDamage = 0;
};

struct GameData
{
	Difficulty difficulty = Difficulty::Normal;
	int score = 0;
	int livesLeft = START_LIVES;
	int playTimeSec = 0;
	int grazeCount = 0;
	bool isClear = false;
	bool isGameOver = false;
	RunStats runStats;
	char rankLetter = 'C';
	float scoreMultiplier = 1.0f;
	int   spellBonusCollected = 0;
};

extern GameData g_GameData;

char ComputeRankLetter(int score, bool isClear, int graze, int maxChain, Difficulty diff);
