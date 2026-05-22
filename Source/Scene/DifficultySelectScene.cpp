#include "Scene/DifficultySelectScene.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/SaveData.h"
#include "Common/KeyHelper.h"
#include "GameConfig.h"
#include "DxLib.h"

void DifficultySelectScene::Init()
{
	m_CursorRow = 0;
	m_ViewAchievements = false;
	m_ModeIdx = 0;
	m_ChapterIdx = 0;
	m_StageIdx = 0;
	m_DiffIdx = 1;
	m_ShotIdx = 0;
	if (m_ModeIdx < 0 || m_ModeIdx > 3) m_ModeIdx = 0;
	if (m_ChapterIdx < 0 || m_ChapterIdx >= STAGE_CHAPTER_COUNT) m_ChapterIdx = 0;
	if (m_StageIdx < 0 || m_StageIdx > 7) m_StageIdx = 0;
	if (m_DiffIdx < 0 || m_DiffIdx >= DifficultyCount()) m_DiffIdx = 1;
	if (m_ShotIdx < 0 || m_ShotIdx > 2) m_ShotIdx = 0;
}

SceneType DifficultySelectScene::Update()
{
	if (KeyHelper::IsCancelTrigger())
		return SceneType::Title;

	if (m_ViewAchievements)
	{
		if (KeyHelper::IsConfirmTrigger() || KeyHelper::IsCancelTrigger())
			m_ViewAchievements = false;
		return SceneType::None;
	}

	const bool isTutorial = (m_ModeIdx == 2);
	const bool isScoreAttack = (m_ModeIdx == 3);
	const int maxRow = 5;

	if (KeyHelper::IsTrigger(KEY_INPUT_UP))
	{
		m_CursorRow--;
		if (m_CursorRow < 0) m_CursorRow = maxRow;
	}
	if (KeyHelper::IsTrigger(KEY_INPUT_DOWN))
	{
		m_CursorRow++;
		if (m_CursorRow > maxRow) m_CursorRow = 0;
	}

	if (KeyHelper::IsTrigger(KEY_INPUT_LEFT))  CycleRow(-1);
	if (KeyHelper::IsTrigger(KEY_INPUT_RIGHT)) CycleRow(1);

	if (KeyHelper::IsConfirmTrigger())
	{
		if (m_CursorRow == maxRow)
			m_ViewAchievements = true;
		else if (isTutorial)
		{
			ApplyTutorialSelection();
			return SceneType::Tutorial;
		}
		else
		{
			ApplySelection();
			return SceneType::Game;
		}
	}

	return SceneType::None;
}

void DifficultySelectScene::CycleRow(int dir)
{
	const bool isTutorial = (m_ModeIdx == 2);
	const bool isScoreAttack = (m_ModeIdx == 3);

	switch (m_CursorRow)
	{
	case 0: m_ModeIdx = (m_ModeIdx + dir + 4) % 4; break;
	case 1:
		if (!isTutorial)
			m_ChapterIdx = (m_ChapterIdx + dir + STAGE_CHAPTER_COUNT) % STAGE_CHAPTER_COUNT;
		break;
	case 2:
		if (!isTutorial && !isScoreAttack)
			m_StageIdx = (m_StageIdx + dir + 8) % 8;
		break;
	case 3:
		if (!isTutorial)
			m_DiffIdx = (m_DiffIdx + dir + DifficultyCount()) % DifficultyCount();
		break;
	case 4: m_ShotIdx = (m_ShotIdx + dir + 3) % 3; break;
	default: break;
	}
}

void DifficultySelectScene::ApplySelection()
{
	switch (m_ModeIdx)
	{
	case 1:  g_Session.playMode = PlayMode::Practice; break;
	case 3:  g_Session.playMode = PlayMode::ScoreAttack; break;
	default: g_Session.playMode = PlayMode::Story; break;
	}

	g_Session.stageChapter = m_ChapterIdx;
	if (g_Session.stageChapter < 0) g_Session.stageChapter = 0;
	if (g_Session.stageChapter >= STAGE_CHAPTER_COUNT)
		g_Session.stageChapter = STAGE_CHAPTER_COUNT - 1;

	if (g_Session.playMode == PlayMode::ScoreAttack)
		g_Session.stageStart = StageStart::All;
	else
	{
		static const StageStart stages[] = {
			StageStart::All, StageStart::Wave1, StageStart::Wave2, StageStart::MidBoss1,
			StageStart::Wave3, StageStart::Wave4, StageStart::MidBoss2, StageStart::BossOnly
		};
		g_Session.stageStart = stages[m_StageIdx];
	}

	g_GameData.difficulty = (Difficulty)m_DiffIdx;
	static const ShotType shots[] = { ShotType::Spread, ShotType::Pierce, ShotType::Homing };
	g_Session.shotType = shots[m_ShotIdx];
}

void DifficultySelectScene::ApplyTutorialSelection()
{
	static const ShotType shots[] = { ShotType::Spread, ShotType::Pierce, ShotType::Homing };
	g_Session.playMode = PlayMode::Tutorial;
	g_Session.shotType = shots[m_ShotIdx];
	g_Session.stageChapter = 0;
}

void DifficultySelectScene::Draw()
{
	int cx = SCREEN_WIDTH / 2;
	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / SCREEN_HEIGHT;
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(6, 4, 32 + (int)(20 * (1 - t))));
	}

	SetFontSize(36);
	DrawFormatString(cx - 200, 40, GetColor(0, 230, 255), "出撃準備メニュー");

	const bool isTutorial = (m_ModeIdx == 2);
	const bool isScoreAttack = (m_ModeIdx == 3);

	const char* modeLabels[] = {
		"ストーリー（通常）",
		"練習モード（無限残機）",
		"チュートリアル（操作練習）",
		"スコアアタック（１面・残機∞）"
	};
	const char* chapterLabels[] = { "第１面", "第２面" };
	const char* stageLabels[] = { "全編", "Ｗ１", "Ｗ２", "中ボス１", "Ｗ３", "Ｗ４", "中ボス２", "ボスのみ" };
	const char* diffLabels[] = { "ＥＡＳＹ", "ＮＯＲＭＡＬ", "ＨＡＲＤ", "ＬＵＮＡＴＩＣ" };
	const char* shotLabels[] = { "拡散弾", "貫通弾", "誘導弾" };
	const char* rowNames[] = { "モード", "面（章）", "ステージ", "難易度", "ショット", "実績・称号" };

	char chapterVal[32];
	if (isTutorial)
		sprintf_s(chapterVal, "―");
	else
		sprintf_s(chapterVal, "%s", chapterLabels[m_ChapterIdx]);

	const char* rowVals[] = {
		modeLabels[m_ModeIdx],
		chapterVal,
		(isTutorial || isScoreAttack) ? "―" : stageLabels[m_StageIdx],
		isTutorial ? "―" : diffLabels[m_DiffIdx],
		shotLabels[m_ShotIdx],
		"一覧を見る"
	};

	for (int i = 0; i <= 5; i++)
	{
		int y = 115 + i * 58;
		bool sel = (m_CursorRow == i);
		if (sel)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 70);
			DrawBox(cx - 400, y - 8, cx + 400, y + 40, GetColor(0, 50, 80), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		SetFontSize(22);
		DrawFormatString(cx - 360, y, sel ? GetColor(255, 255, 0) : GetColor(180, 180, 200),
			"%s", rowNames[i]);
		SetFontSize(24);
		DrawFormatString(cx + 40, y, sel ? GetColor(255, 255, 255) : GetColor(0, 255, 200),
			"%s", rowVals[i]);
	}

	SetFontSize(18);
	DrawFormatString(cx - 280, SCREEN_HEIGHT - 70, GetColor(160, 160, 180),
		"練習／スコアアタック中は Q でショット切替");
	DrawFormatString(cx - 200, SCREEN_HEIGHT - 45, GetColor(160, 160, 180),
		"ENTER:決定  ESC:戻る");

	if (m_ViewAchievements)
		DrawAchievementsOverlay();
}

void DifficultySelectScene::DrawAchievementsOverlay()
{
	int cx = SCREEN_WIDTH / 2;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(80, 60, SCREEN_WIDTH - 80, SCREEN_HEIGHT - 60, GetColor(10, 10, 30), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetFontSize(32);
	DrawFormatString(cx - 80, 80, GetColor(255, 215, 0), "実績一覧");

	SetFontSize(18);
	int y = 130;
	for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT; i++)
	{
		bool unlocked = g_Save.achievements[i];
		unsigned int col = unlocked ? GetColor(0, 255, 180) : GetColor(100, 100, 120);
		DrawFormatString(120, y, col, "%s %s",
			unlocked ? "[○]" : "[×]",
			SaveDataGetAchievementName((AchievementId)i));
		y += 28;
		if (y > SCREEN_HEIGHT - 100) break;
	}

	SetFontSize(20);
	DrawFormatString(cx - 120, SCREEN_HEIGHT - 90, GetColor(200, 200, 200), "ENTER / ESC で戻る");
}
