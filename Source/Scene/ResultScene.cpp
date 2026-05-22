#include "Scene/ResultScene.h"
#include "GameConfig.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/SaveData.h"
#include "Common/KeyHelper.h"
#include "DxLib.h"

void ResultScene::Init()
{
	m_AnimTimer = 0;
	FinalizeAchievements();
}

void ResultScene::FinalizeAchievements()
{
	if (g_GameData.isClear)
	{
		SaveDataUnlockAchievement(AchievementId::FirstClear);
		if (g_Session.playMode == PlayMode::Practice)
			SaveDataUnlockAchievement(AchievementId::PracticeClear);
		if (g_Session.shotType == ShotType::Homing)
			SaveDataUnlockAchievement(AchievementId::HomingMaster);
	}
	if (g_GameData.grazeCount >= 100) SaveDataUnlockAchievement(AchievementId::Graze100);
	if (g_GameData.grazeCount >= 500) SaveDataUnlockAchievement(AchievementId::Graze500);
	if (g_GameData.runStats.maxKillChain >= 10) SaveDataUnlockAchievement(AchievementId::KillChain10);
	if (g_GameData.runStats.feverActivations >= 5) SaveDataUnlockAchievement(AchievementId::Fever5);
	if (g_GameData.runStats.bossMaxPhaseReached >= 2) SaveDataUnlockAchievement(AchievementId::BossPhase3);
	if (g_GameData.score >= 500000) SaveDataUnlockAchievement(AchievementId::Score500k);
}

SceneType ResultScene::Update()
{
	m_AnimTimer++;
	if (KeyHelper::IsCancelTrigger()) return SceneType::Exit;
	if (KeyHelper::IsConfirmTrigger()) return SceneType::Title;
	return SceneType::None;
}

void ResultScene::Draw()
{
	const int centerX = SCREEN_WIDTH / 2;
	const int lineH = 30;

	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / (float)SCREEN_HEIGHT;
		int r = (int)(g_GameData.isClear ? 12 * (1.0f - t) : 30 * (1.0f - t));
		int g = (int)(8 * (1.0f - t));
		int b = (int)(g_GameData.isClear ? 50 + 22 * (1.0f - t) : 24 + 8 * (1.0f - t));
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(r, g, b));
	}

	int topY = 48;
	SetFontSize(48);
	unsigned int titleColor = g_GameData.isClear ? GetColor(255, 215, 0) : GetColor(255, 60, 60);
	const char* titleStr = g_GameData.isClear ? "ミッションクリア！" : "ミッション失敗…";
	DrawFormatString(centerX - 220, topY, titleColor, "%s", titleStr);

	topY += 72;
	SetFontSize(64);
	unsigned int rankCol = GetColor(255, 255, 255);
	if (g_GameData.rankLetter == 'S') rankCol = GetColor(255, 215, 0);
	else if (g_GameData.rankLetter == 'A') rankCol = GetColor(200, 255, 200);
	DrawFormatString(centerX - 50, topY, rankCol, "ランク %c", g_GameData.rankLetter);

	const char* diffLabels[] = { "ＥＡＳＹ", "ＮＯＲＭＡＬ", "ＨＡＲＤ", "ＬＵＮＡＴＩＣ" };
	int dIdx = (int)g_GameData.difficulty;
	if (dIdx < 0 || dIdx >= DifficultyCount()) dIdx = 1;

	int statY = topY + 90;
	const int statX = centerX - 220;
	SetFontSize(22);

	DrawFormatString(statX, statY, GetColor(255, 255, 255), "スコア      :  %08d", g_GameData.score);
	statY += lineH;
	DrawFormatString(statX, statY, GetColor(200, 200, 250), "難易度      :  %s", diffLabels[dIdx]);
	statY += lineH;
	DrawFormatString(statX, statY, GetColor(255, 200, 100), "グレイズ    :  %d 回", g_GameData.grazeCount);
	statY += lineH;
	int min = g_GameData.playTimeSec / 60;
	int sec = g_GameData.playTimeSec % 60;
	DrawFormatString(statX, statY, GetColor(200, 200, 200), "プレイ時間  :  %02d:%02d", min, sec);
	statY += lineH + 8;

	DrawFormatString(statX, statY, GetColor(0, 255, 200), "最大連続撃破:  %d", g_GameData.runStats.maxKillChain);
	statY += lineH;
	DrawFormatString(statX, statY, GetColor(255, 180, 220), "フィーバー  :  %d 回", g_GameData.runStats.feverActivations);
	statY += lineH;
	DrawFormatString(statX, statY, GetColor(180, 220, 255), "ボム使用    :  %d 回", g_GameData.runStats.bombsUsed);
	statY += lineH;
	DrawFormatString(statX, statY, GetColor(200, 255, 180), "撃破数      :  %d", g_GameData.runStats.enemiesDestroyed);
	statY += lineH;

	if (g_GameData.scoreMultiplier > 1.01f)
	{
		DrawFormatString(statX, statY, GetColor(255, 180, 255), "ボーナス倍率:  x%.2f", g_GameData.scoreMultiplier);
		statY += lineH;
	}

	int hi = (g_Session.playMode == PlayMode::ScoreAttack)
		? g_Save.scoreAttackBest[dIdx]
		: g_Save.highScore[dIdx];
	const char* hiLabel = (g_Session.playMode == PlayMode::ScoreAttack) ? "SCアタック" : "ハイスコア";
	DrawFormatString(statX, statY, GetColor(255, 255, 150), "%s(%s): %08d", hiLabel, diffLabels[dIdx], hi);

	SetFontSize(20);
	DrawFormatString(statX, SCREEN_HEIGHT - 120, GetColor(255, 200, 100),
		"称号: %s", SaveDataGetActiveTitle());

	if ((m_AnimTimer / 30) % 2 == 0)
	{
		SetFontSize(22);
		DrawFormatString(centerX - 200, SCREEN_HEIGHT - 52, GetColor(0, 255, 200),
			">> ENTERでタイトルへ <<");
	}
}
