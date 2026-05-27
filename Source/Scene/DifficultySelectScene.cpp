#include "Scene/DifficultySelectScene.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/SaveData.h"
#include "Common/KeyHelper.h"
#include "Common/UiDraw.h"
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
	m_StageNumberIdx = 0;
	if (m_ModeIdx < 0 || m_ModeIdx > 3) m_ModeIdx = 0;
	if (m_ChapterIdx < 0 || m_ChapterIdx >= STAGE_CHAPTER_COUNT) m_ChapterIdx = 0;
	if (m_StageIdx < 0 || m_StageIdx > 7) m_StageIdx = 0;
	if (m_DiffIdx < 0 || m_DiffIdx >= DifficultyCount()) m_DiffIdx = 1;
	if (m_PhaseStartIdx < 0 || m_PhaseStartIdx >= BOSS_RUSH_PHASE_COUNT) m_PhaseStartIdx = 0;
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
	const int maxRow = BOSS_RUSH_MODE ? 5 : 5;

	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_UP))
	{
		m_CursorRow--;
		if (m_CursorRow < 0) m_CursorRow = maxRow;
	}
	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_DOWN))
	{
		m_CursorRow++;
		if (m_CursorRow > maxRow) m_CursorRow = 0;
	}

	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_LEFT))  CycleRow(-1);
	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_RIGHT)) CycleRow(1);

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
			return (g_Session.stageNumber == 2) ? SceneType::Stage2 : SceneType::Game;
		}
	}

	return SceneType::None;
}

void DifficultySelectScene::CycleRow(int dir)
{
	const bool isTutorial = (m_ModeIdx == 2);
	const bool isScoreAttack = (m_ModeIdx == 3);

	if (BOSS_RUSH_MODE)
	{
		switch (m_CursorRow)
		{
		case 0: m_ModeIdx = (m_ModeIdx + dir + 4) % 4; break;
		case 1:
			if (!isTutorial)
				m_StageNumberIdx = (m_StageNumberIdx + dir + 2) % 2;
			break;
		case 2:
			if (!isTutorial)
				m_DiffIdx = (m_DiffIdx + dir + DifficultyCount()) % DifficultyCount();
			break;
		case 3:
			// 開始形態：一面・二面ともボス戦なので両方で有効
			if (!isTutorial)
				m_PhaseStartIdx = (m_PhaseStartIdx + dir + BOSS_RUSH_PHASE_COUNT) % BOSS_RUSH_PHASE_COUNT;
			break;
		case 4: m_ShotIdx = (m_ShotIdx + dir + 3) % 3; break;
		default: break;
		}
		return;
	}

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

	if (BOSS_RUSH_MODE)
	{
		g_Session.stageChapter = 0;
		g_Session.stageStart = StageStart::BossOnly;
		g_Session.bossRushStartPhase = ClampBossRushStartPhase(m_PhaseStartIdx);
		g_Session.stageNumber = (m_StageNumberIdx == 1) ? 2 : 1;
	}
	else
	{
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

		g_Session.stageNumber = 1;
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
	g_Session.bossRushStartPhase = 0;
	g_Session.stageNumber = 1;
}

void DifficultySelectScene::DrawPrepMenuRows(int labelX, int valueX, int startY, int rowH, int rowCount,
	const char* const* names, const char* const* values) const
{
	for (int i = 0; i < rowCount; i++)
	{
		const int y = startY + i * rowH;
		const bool sel = (m_CursorRow == i);

		if (sel)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 72);
			DrawBox(labelX - 14, y - 8, valueX + 400, y + rowH - 12, GetColor(0, 50, 80), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		SetFontSize(26);
		const unsigned int nameCol = sel ? GetColor(255, 255, 0) : GetColor(180, 190, 210);
		DrawFormatString(labelX, y, nameCol, "%s", names[i]);

		SetFontSize(28);
		const unsigned int valCol = sel ? GetColor(255, 255, 255) : GetColor(0, 255, 200);
		DrawFormatString(valueX, y, valCol, "%s", values[i]);
	}
}

void DifficultySelectScene::DrawBossRushInfoPanel(int left, int top, int right, int bottom,
	Difficulty diff, bool practiceMode, int startPhase, bool isTutorial) const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 52);
	DrawBox(left, top, right, bottom, GetColor(10, 14, 36), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(left, top, right, bottom, GetColor(0, 120, 170), FALSE);

	const int padX = 20;
	int y = top + 16;

	SetFontSize(24);
	DrawFormatString(left + padX, y, GetColor(0, 220, 255), "ボス情報");
	y += 36;

	if (isTutorial)
	{
		SetFontSize(20);
		DrawTextUtf8Centered((left + right) / 2, top + (bottom - top) / 2 - 20,
			GetColor(180, 190, 220), "チュートリアルでは操作練習のみです");
		return;
	}

	const char* bossName = GetBossDisplayName(0);
	SetFontSize(26);
	DrawTextUtf8(left + padX, y, GetColor(255, 200, 120), bossName);
	y += 38;

	static const char* phaseShort[] = { "第１形態", "第２形態", "第３形態", "第４形態" };
	int totalHp = 0;
	int maxPhaseHp = 1;

	for (int p = 0; p < BOSS_RUSH_PHASE_COUNT; p++)
	{
		const int hp = GetBossRushPhaseHp(diff, p, practiceMode);
		totalHp += hp;
		if (hp > maxPhaseHp) maxPhaseHp = hp;
	}

	const int barMaxW = (right - left) - padX * 2 - 8;
	const int phaseListTop = y;
	const int statsBlockH = 118;
	const int statsTop = bottom - statsBlockH - 12;
	const int phaseAreaH = statsTop - 14 - phaseListTop;
	int rowH = phaseAreaH / BOSS_RUSH_PHASE_COUNT;
	if (rowH < 46) rowH = 46;
	if (rowH > 56) rowH = 56;
	startPhase = ClampBossRushStartPhase(startPhase);

	y = phaseListTop;
	for (int p = 0; p < BOSS_RUSH_PHASE_COUNT; p++)
	{
		const bool highlight = (p == startPhase);
		const int hp = GetBossRushPhaseHp(diff, p, practiceMode);
		const char* spell = GetSpellCardName(0, p);

		if (highlight)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 90);
			DrawBox(left + 10, y - 4, right - 10, y + rowH - 8, GetColor(40, 20, 70), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}

		SetFontSize(20);
		const unsigned int phaseCol = highlight ? GetColor(255, 255, 120) : GetColor(200, 200, 220);
		DrawFormatString(left + padX, y, phaseCol, "%s", phaseShort[p]);

		SetFontSize(18);
		DrawFormatString(left + padX + 92, y + 1, highlight ? GetColor(255, 220, 255) : GetColor(180, 200, 255),
			"%s", spell);

		SetFontSize(20);
		DrawFormatString(right - padX - 76, y, highlight ? GetColor(255, 180, 100) : GetColor(160, 200, 180),
			"HP %d", hp);

		const int barW = barMaxW * hp / maxPhaseHp;
		const int barY = y + rowH - 22;
		unsigned int barBg = GetColor(30, 40, 60);
		unsigned int barFg = highlight ? GetColor(255, 100, 140) : GetColor(80, 160, 200);
		DrawBox(left + padX, barY, left + padX + barMaxW, barY + 6, barBg, TRUE);
		if (barW > 0)
			DrawBox(left + padX, barY, left + padX + barW, barY + 6, barFg, TRUE);

		if (highlight)
		{
			SetFontSize(16);
			DrawFormatString(right - padX - 92, y + rowH - 18, GetColor(255, 255, 160), "▶ 開始形態");
		}

		y += rowH;
	}

	static const char* diffLabels[] = { "ＥＡＳＹ", "ＮＯＲＭＡＬ", "ＨＡＲＤ", "ＬＵＮＡＴＩＣ" };
	const int diffIdx = (int)diff;
	const BossRushDifficultyTune& tune = GetBossRushTune(diff);

	DrawLine(left + 14, statsTop - 10, right - 14, statsTop - 10, GetColor(0, 100, 140));

	int sy = statsTop;
	SetFontSize(18);
	DrawFormatString(left + padX, sy, GetColor(200, 220, 255),
		"全形態合計 HP 約 %d", totalHp);
	sy += 26;

	DrawFormatString(left + padX, sy, GetColor(180, 200, 230),
		"難易度 %s　%s",
		diffLabels[diffIdx >= 0 && diffIdx < 4 ? diffIdx : 1],
		practiceMode ? "練習モード" : "通常プレイ");
	sy += 26;

	DrawFormatString(left + padX, sy, GetColor(160, 180, 210),
		"弾速 x%.0f%%　密度 x%.0f%%",
		tune.bulletSpeedMul * 100.0f, tune.bulletCountMul * 100.0f);
	sy += 26;

	const int best = g_Save.highScore[diffIdx >= 0 && diffIdx < 4 ? diffIdx : 0];
	DrawFormatString(left + padX, sy, GetColor(255, 215, 100),
		"ベストスコア  %08d", best);
}

void DifficultySelectScene::Draw()
{
	const int cx = SCREEN_WIDTH / 2;
	const int layoutShiftY = SCREEN_HEIGHT * 4 / 100;

	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / SCREEN_HEIGHT;
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(6, 4, 32 + (int)(20 * (1 - t))));
	}

	const bool isTutorial = (m_ModeIdx == 2);
	const bool isScoreAttack = (m_ModeIdx == 3);

	static const char* modeLabelsRush[] = {
		"ボスラッシュ（通常）",
		"練習（無限残機・弾緩め）",
		"チュートリアル（操作練習）",
		"スコアアタック（ランキング）"
	};
	static const char* modeLabelsStory[] = {
		"ストーリー（通常）",
		"練習モード（無限残機）",
		"チュートリアル（操作練習）",
		"スコアアタック（１面・残機∞）"
	};
	const char* const* modeLabels = BOSS_RUSH_MODE ? modeLabelsRush : modeLabelsStory;
	const char* chapterLabels[] = { "第１面", "第２面" };
	const char* stageLabels[] = { "全編", "Ｗ１", "Ｗ２", "中ボス１", "Ｗ３", "Ｗ４", "中ボス２", "ボスのみ" };
	const char* diffLabels[] = { "ＥＡＳＹ", "ＮＯＲＭＡＬ", "ＨＡＲＤ", "ＬＵＮＡＴＩＣ" };
	const char* shotLabels[] = { "拡散弾", "貫通弾", "誘導弾" };

	const char* titleText = BOSS_RUSH_MODE ? "ボスラッシュ　出撃準備" : "出撃準備メニュー";
	const int titleY = SCREEN_HEIGHT * 7 / 100 + layoutShiftY;

	SetFontSize(40);
	DrawTextUtf8Centered(cx, titleY, GetColor(0, 230, 255), titleText);

	const int titleH = GetTextDrawHeight(titleText);
	const int sepY = titleY + titleH + 10;
	const int titleW = GetTextDrawWidth(titleText);
	DrawLine(cx - titleW / 2 - 16, sepY, cx + titleW / 2 + 16, sepY, GetColor(0, 180, 220));

	const char* rowNamesBoss[] = { "モード", "ステージ", "難易度", "開始形態", "ショット", "実績・称号" };
	const char* rowNamesFull[] = { "モード", "面（章）", "ステージ", "難易度", "ショット", "実績・称号" };
	const int rowCount = BOSS_RUSH_MODE ? 6 : 6;
	const int rowH = SCREEN_HEIGHT * 8 / 100;
	const int menuBlockH = rowCount * rowH;
	const int menuStartY = SCREEN_HEIGHT / 2 - menuBlockH / 2 + layoutShiftY;

	char chapterVal[32];
	if (isTutorial)
		sprintf_s(chapterVal, "―");
	else
		sprintf_s(chapterVal, "%s", chapterLabels[m_ChapterIdx]);

	static const char* stageNumberLabels[] = {
		"Ｓｔａｇｅ １（弾幕ボスラッシュ）",
		"Ｓｔａｇｅ ２（全方位３Ｄ射撃）"
	};
	const int stageNumIdxClamped = (m_StageNumberIdx == 1) ? 1 : 0;
	const bool isStage2 = (stageNumIdxClamped == 1);

	const char* rowValsBoss[] = {
		modeLabels[m_ModeIdx],
		isTutorial ? "―" : stageNumberLabels[stageNumIdxClamped],
		isTutorial ? "―" : diffLabels[m_DiffIdx],
		isTutorial ? "―" : GetBossRushPhaseStartLabel(m_PhaseStartIdx),
		shotLabels[m_ShotIdx],
		"一覧を見る"
	};
	const char* rowValsFull[] = {
		modeLabels[m_ModeIdx],
		chapterVal,
		(isTutorial || isScoreAttack) ? "―" : stageLabels[m_StageIdx],
		isTutorial ? "―" : diffLabels[m_DiffIdx],
		shotLabels[m_ShotIdx],
		"一覧を見る"
	};

	if (BOSS_RUSH_MODE)
	{
		const int labelX = SCREEN_WIDTH * 6 / 100;
		const int valueX = SCREEN_WIDTH * 20 / 100;
		const int panelL = SCREEN_WIDTH * 50 / 100;
		const int panelR = SCREEN_WIDTH - SCREEN_WIDTH * 4 / 100;
		const int panelT = menuStartY - 16;
		const int minPanelH = 16 + 36 + 38 + BOSS_RUSH_PHASE_COUNT * 48 + 118 + 28;
		int panelB = panelT + minPanelH;
		const int panelMaxB = SCREEN_HEIGHT - 76;
		if (panelB > panelMaxB)
			panelB = panelMaxB;
		if (panelB < menuStartY + menuBlockH + 8)
			panelB = menuStartY + menuBlockH + 8;

		DrawPrepMenuRows(labelX, valueX, menuStartY, rowH, rowCount, rowNamesBoss, rowValsBoss);

		const bool practice = (m_ModeIdx == 1);
		if (isStage2 && !isTutorial)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 52);
			DrawBox(panelL, panelT, panelR, panelB, GetColor(10, 14, 36), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawBox(panelL, panelT, panelR, panelB, GetColor(0, 200, 150), FALSE);

			const int padX = 20;
			int y = panelT + 16;
			SetFontSize(24);
			DrawFormatString(panelL + padX, y, GetColor(0, 255, 200), "ステージ２　ボス戦（全方位３Ｄ）");
			y += 36;
			SetFontSize(18);
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ ＴＰＳ視点：自機を後方から追従"); y += 24;
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ マウス：視点・照準"); y += 24;
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ Ｓｐａｃｅ：連射　Ｘ/Ｂ：ボム"); y += 24;
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ ＷＡＳＤ：前後・左右（視線基準）"); y += 24;
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ Ｅ/Ｒ：上昇　Ｑ/Ｆ：下降"); y += 24;
			DrawFormatString(panelL + padX, y, GetColor(220, 240, 255), "■ Ｓｈｉｆｔ：低速・精密照準"); y += 30;
			SetFontSize(17);
			DrawFormatString(panelL + padX, y, GetColor(200, 220, 200),
				"ボスは３Ｄ空間を全方向に飛び回ります。"); y += 22;
			DrawFormatString(panelL + padX, y, GetColor(200, 220, 200),
				"視線をボスへ合わせて撃破せよ。"); y += 28;

			SetFontSize(18);
			DrawFormatString(panelL + padX, y, GetColor(255, 215, 120),
				"ボス：%s", GetBossDisplayName(0)); y += 26;
			for (int p = 0; p < BOSS_RUSH_PHASE_COUNT; p++)
			{
				const int hp = GetBossRushPhaseHp((Difficulty)m_DiffIdx, p, practice);
				const bool hi = (p == m_PhaseStartIdx);
				unsigned int col = hi ? GetColor(255, 255, 160) : GetColor(180, 200, 230);
				DrawFormatString(panelL + padX, y, col,
					"%s 第%d形態  HP %d",
					hi ? "▶" : "  ", p + 1, hp);
				y += 22;
			}
		}
		else
		{
			DrawBossRushInfoPanel(panelL, panelT, panelR, panelB,
				(Difficulty)m_DiffIdx, practice, m_PhaseStartIdx, isTutorial);
		}

		if (!isTutorial)
		{
			SetFontSize(17);
			DrawTextUtf8Centered(cx, SCREEN_HEIGHT - 72, GetColor(140, 160, 200),
				isStage2
				? "ステージ２：マウスで照準 ／ Ｓｐａｃｅで射撃"
				: "開始形態で第1〜4形態を個別に確認できます");
		}
	}
	else
	{
		const int labelX = cx - 380;
		const int valueX = cx + 20;
		DrawPrepMenuRows(labelX, valueX, menuStartY, rowH, rowCount, rowNamesFull, rowValsFull);
	}

	SetFontSize(18);
	DrawTextUtf8Centered(cx, SCREEN_HEIGHT - 44, GetColor(160, 160, 180),
		"練習／SC中は Q でショット切替");
	SetFontSize(17);
	DrawTextUtf8Centered(cx, SCREEN_HEIGHT - 22, GetColor(160, 160, 180),
		"ENTER:決定  ESC:戻る");

	SetFontSize(20);

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
	DrawTextUtf8Centered(cx, 80, GetColor(255, 215, 0), "実績一覧");

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
	DrawTextUtf8Centered(cx, SCREEN_HEIGHT - 90, GetColor(200, 200, 200), "ENTER / ESC で戻る");
}
