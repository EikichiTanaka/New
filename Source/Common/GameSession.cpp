#include "Common/GameSession.h"

GameSession g_Session;

int StageStartToWaveIndex(StageStart stage)
{
	switch (stage)
	{
	case StageStart::Wave1:    return 0;
	case StageStart::Wave2:    return 1;
	case StageStart::MidBoss1: return 1;
	case StageStart::Wave3:    return 2;
	case StageStart::Wave4:    return 3;
	case StageStart::MidBoss2: return 3;
	case StageStart::BossOnly: return 4;
	default:                   return 0;
	}
}

bool StageStartSkipsToBoss(StageStart stage)
{
	return stage == StageStart::BossOnly;
}

bool StageStartIsMidBossOnly(StageStart stage, int& outMidBossId)
{
	if (stage == StageStart::MidBoss1) { outMidBossId = 1; return true; }
	if (stage == StageStart::MidBoss2) { outMidBossId = 2; return true; }
	return false;
}
