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
	// ボスラッシュ：開始形態（0=第1形態 … 3=第4形態）。確認・練習用
	int        bossRushStartPhase = 0;

	// 出撃する面（1=一面：弾幕ボスラッシュ、2=二面：全方位3Dシューター）
	int        stageNumber   = 1;

	// ポーズ中オプションから戻るとき true（OptionsScene が参照）
	bool       returnToGameAfterOptions = false;
};

extern GameSession g_Session;

int StageStartToWaveIndex(StageStart stage);
bool StageStartSkipsToBoss(StageStart stage);
bool StageStartIsMidBossOnly(StageStart stage, int& outMidBossId);
