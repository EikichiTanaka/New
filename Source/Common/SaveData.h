#pragma once
#include "GameConfig.h"

enum class AchievementId : int
{
	FirstClear = 0,
	Graze100,
	Graze500,
	KillChain10,
	Fever5,
	BossPhase3,
	PracticeClear,
	HomingMaster,
	NoHitWave,
	Score500k,
	ACHIEVEMENT_COUNT
};

struct SaveData
{
	int highScore[4];
	int scoreAttackBest[4];
	bool achievements[(int)AchievementId::ACHIEVEMENT_COUNT];
	char titles[64];
	int saveVersion;
};

constexpr int SAVE_DATA_VERSION = 1;

void SaveDataLoad();
void SaveDataSave();
void SaveDataTryUpdateHighScore(Difficulty diff, int score);
void SaveDataTryUpdateScoreAttack(Difficulty diff, int score, int playTimeSec);
bool SaveDataUnlockAchievement(AchievementId id);
const char* SaveDataGetAchievementName(AchievementId id);
const char* SaveDataGetAchievementDesc(AchievementId id);
const char* SaveDataGetActiveTitle();

extern SaveData g_Save;
