#include "Common/SaveData.h"
#include "DxLib.h"
#include <cstdio>
#include <cstring>

SaveData g_Save;

static const char* SAVE_PATH = "savegame.dat";
static const char* DEFAULT_TITLE = "見習い弾幕使い";

struct SaveDataV1
{
	int highScore[4];
	int scoreAttackBest[4];
	bool achievements[(int)AchievementId::ACHIEVEMENT_COUNT];
	char titles[64];
};

void SaveDataLoad()
{
	g_Save = {};
	g_Save.saveVersion = SAVE_DATA_VERSION;

	FILE* fp = nullptr;
	if (fopen_s(&fp, SAVE_PATH, "rb") == 0 && fp)
	{
		fseek(fp, 0, SEEK_END);
		long sz = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		if (sz >= (long)sizeof(SaveData))
			fread(&g_Save, sizeof(SaveData), 1, fp);
		else if (sz >= (long)sizeof(SaveDataV1))
		{
			SaveDataV1 legacy = {};
			fread(&legacy, sizeof(SaveDataV1), 1, fp);
			memcpy(g_Save.highScore, legacy.highScore, sizeof(legacy.highScore));
			memcpy(g_Save.scoreAttackBest, legacy.scoreAttackBest, sizeof(legacy.scoreAttackBest));
			memcpy(g_Save.achievements, legacy.achievements, sizeof(legacy.achievements));
			strcpy_s(g_Save.titles, legacy.titles);
		}
		fclose(fp);
	}

	if (g_Save.titles[0] == '\0')
		strcpy_s(g_Save.titles, DEFAULT_TITLE);
}

void SaveDataSave()
{
	g_Save.saveVersion = SAVE_DATA_VERSION;
	FILE* fp = nullptr;
	if (fopen_s(&fp, SAVE_PATH, "wb") == 0 && fp)
	{
		fwrite(&g_Save, sizeof(SaveData), 1, fp);
		fclose(fp);
	}
}

void SaveDataTryUpdateHighScore(Difficulty diff, int score)
{
	int idx = (int)diff;
	if (idx < 0 || idx > 3) return;
	if (score > g_Save.highScore[idx])
	{
		g_Save.highScore[idx] = score;
		SaveDataSave();
	}
}

void SaveDataTryUpdateScoreAttack(Difficulty diff, int score, int playTimeSec)
{
	(void)playTimeSec;
	int idx = (int)diff;
	if (idx < 0 || idx > 3) return;
	if (score > g_Save.scoreAttackBest[idx])
	{
		g_Save.scoreAttackBest[idx] = score;
		SaveDataSave();
	}
}

bool SaveDataUnlockAchievement(AchievementId id)
{
	int i = (int)id;
	if (i < 0 || i >= (int)AchievementId::ACHIEVEMENT_COUNT) return false;
	if (g_Save.achievements[i]) return false;
	g_Save.achievements[i] = true;
	SaveDataSave();
	return true;
}

const char* SaveDataGetAchievementName(AchievementId id)
{
	switch (id)
	{
	case AchievementId::FirstClear:    return "初クリア";
	case AchievementId::Graze100:      return "かすり職人";
	case AchievementId::Graze500:      return "かすり達人";
	case AchievementId::KillChain10:   return "連撃王";
	case AchievementId::Fever5:        return "フィーバー魔";
	case AchievementId::BossPhase3:    return "ボス討伐者";
	case AchievementId::PracticeClear: return "練習完了";
	case AchievementId::HomingMaster:  return "誘導の極意";
	case AchievementId::NoHitWave:     return "無傷ウェーブ";
	case AchievementId::Score500k:     return "ハイスコア";
	default:                           return "？？？";
	}
}

const char* SaveDataGetAchievementDesc(AchievementId id)
{
	switch (id)
	{
	case AchievementId::FirstClear:    return "ゲームをクリアする";
	case AchievementId::Graze100:      return "累計グレイズ100回";
	case AchievementId::Graze500:      return "累計グレイズ500回";
	case AchievementId::KillChain10:   return "10連続撃破を達成";
	case AchievementId::Fever5:        return "フィーバー5回発動";
	case AchievementId::BossPhase3:    return "ボス最終フェーズ到達";
	case AchievementId::PracticeClear: return "練習モードでクリア";
	case AchievementId::HomingMaster:  return "誘導弾でクリア";
	case AchievementId::NoHitWave:     return "1ウェーブ無被弾クリア";
	case AchievementId::Score500k:     return "スコア500000点";
	default:                           return "";
	}
}

const char* SaveDataGetActiveTitle()
{
	int unlocked = 0;
	for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT; i++)
		if (g_Save.achievements[i]) unlocked++;

	if (unlocked >= 8) return "伝説の弾幕使い";
	if (unlocked >= 5) return "弾幕カグラ";
	if (unlocked >= 3) return "弾幕名人";
	if (g_Save.achievements[(int)AchievementId::FirstClear]) return "クリアの勇者";
	return g_Save.titles;
}
