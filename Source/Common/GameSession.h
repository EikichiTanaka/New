#pragma once
#include "GameConfig.h"

enum class PlayMode
{
	Story = 0,
	Practice,
	Tutorial,
	ScoreAttack
};

enum class StageStart
{
	All = 0,
	Wave1,
	Wave2,
	MidBoss1,
	Wave3,
	Wave4,
	MidBoss2,
	BossOnly
};

enum class ShotType
{
	Spread = 0,
	Pierce,
	Homing
};

struct GameSession
{
	PlayMode   playMode      = PlayMode::Story;
	StageStart stageStart    = StageStart::All;
	ShotType   shotType      = ShotType::Spread;
	int        stageChapter  = 0;
};

extern GameSession g_Session;

int StageStartToWaveIndex(StageStart stage);
bool StageStartSkipsToBoss(StageStart stage);
bool StageStartIsMidBossOnly(StageStart stage, int& outMidBossId);
