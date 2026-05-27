#include "Scene/ResultScene.h"
#include "GameConfig.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/SaveData.h"
#include "Common/KeyHelper.h"
#include "Common/UiDraw.h"
#include "DxLib.h"
#include <cmath>
#include <cstring>

namespace
{
constexpr int   ROLL_START_DELAY = 12;
constexpr float ROLL_SMOOTH      = 0.048f;
}

void ResultScene::Init()
{
	m_AnimTimer = 0;
	m_Roll = {};
	m_MenuCursor = 0;
	m_IsNewRecord = false;
	for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT; i++)
		m_JustUnlocked[i] = false;

	FinalizeAchievements();

	int dIdx = (int)g_GameData.difficulty;
	if (dIdx < 0 || dIdx >= DifficultyCount()) dIdx = 1;
	if (g_Session.playMode == PlayMode::ScoreAttack)
		m_IsNewRecord = g_GameData.isClear
			&& g_GameData.score >= g_Save.scoreAttackBest[dIdx] && g_GameData.score > 0;
	else if (g_Session.playMode != PlayMode::Practice && g_Session.playMode != PlayMode::Tutorial)
		m_IsNewRecord = g_GameData.score >= g_Save.highScore[dIdx] && g_GameData.score > 0;
}

void ResultScene::FinalizeAchievements()
{
	auto tryUnlock = [this](AchievementId id)
	{
		if (SaveDataUnlockAchievement(id))
			m_JustUnlocked[(int)id] = true;
	};

	if (g_GameData.isClear)
	{
		tryUnlock(AchievementId::FirstClear);
		if (g_Session.playMode == PlayMode::Practice)
			tryUnlock(AchievementId::PracticeClear);
		if (g_Session.shotType == ShotType::Homing)
			tryUnlock(AchievementId::HomingMaster);
	}
	if (g_GameData.grazeCount >= 100) tryUnlock(AchievementId::Graze100);
	if (g_GameData.grazeCount >= 500) tryUnlock(AchievementId::Graze500);
	if (g_GameData.runStats.maxKillChain >= 10) tryUnlock(AchievementId::KillChain10);
	if (g_GameData.runStats.feverActivations >= 5) tryUnlock(AchievementId::Fever5);
	if (g_GameData.runStats.bossMaxPhaseReached >= 2) tryUnlock(AchievementId::BossPhase3);
	if (g_GameData.score >= 500000) tryUnlock(AchievementId::Score500k);
}

int ResultScene::CountJustUnlocked() const
{
	int n = 0;
	for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT; i++)
		if (m_JustUnlocked[i]) n++;
	return n;
}

bool ResultScene::HasClearBonus() const
{
	if (!g_GameData.isClear)
		return false;
	return g_GameData.runStats.noBombUsed
		|| g_GameData.runStats.clearedWithoutDamage
		|| g_GameData.runStats.wavesNoDamage > 0;
}

void ResultScene::ComputeLayout(ResultUILayout& L) const
{
	const int layoutShiftY = SCREEN_HEIGHT * 2 / 100;

	L.headerTitleY = SCREEN_HEIGHT * 5 / 100 + layoutShiftY;
	SetFontSize(44);
	L.sepY = L.headerTitleY + GetTextDrawHeight("ミッションクリア！") + 8;

	L.menuBoxTop = SCREEN_HEIGHT - 98;
	const int hintH = 22;

	int y = L.menuBoxTop - 8;

	y -= hintH;
	y -= 8;

	SetFontSize(22);
	char titleLine[96];
	sprintf_s(titleLine, "称号  %s", SaveDataGetActiveTitle());
	L.playerTitleY = y - GetTextDrawHeight(titleLine);
	y = L.playerTitleY - 10;

	const int achCount = CountJustUnlocked();
	L.showAchievements = achCount > 0;
	if (L.showAchievements)
	{
		const int achBlockH = (achCount >= 3) ? 58 : 52;
		L.achY = y - achBlockH;
		y = L.achY - 10;
	}
	else
	{
		L.achY = 0;
	}

	L.showBonus = HasClearBonus();
	if (L.showBonus)
	{
		const int bonusBlockH = 48;
		L.bonusY = y - bonusBlockH;
		y = L.bonusY - 12;
	}
	else
	{
		L.bonusY = 0;
	}

	L.footerPanelBottom = L.menuBoxTop - 8;
	L.footerPanelTop = y;
	if (L.footerPanelTop > L.footerPanelBottom - 8)
		L.footerPanelTop = L.footerPanelBottom - 8;

	L.mainPanelTop = L.sepY + 14;
	L.mainPanelBottom = L.footerPanelTop - 14;

	const int minMainH = 220;
	if (L.mainPanelBottom - L.mainPanelTop < minMainH)
		L.mainPanelBottom = L.mainPanelTop + minMainH;

	if (L.mainPanelBottom > L.footerPanelTop - 8)
		L.mainPanelBottom = L.footerPanelTop - 8;
}

void ResultScene::UpdateRollAnimations()
{
	if (m_AnimTimer < ROLL_START_DELAY)
		return;

	int dIdx = (int)g_GameData.difficulty;
	if (dIdx < 0 || dIdx >= DifficultyCount()) dIdx = 1;
	const int hiTarget = (g_Session.playMode == PlayMode::ScoreAttack)
		? g_Save.scoreAttackBest[dIdx]
		: g_Save.highScore[dIdx];

	auto step = [](float& current, float target)
	{
		if (current == target)
			return;
		current += (target - current) * ROLL_SMOOTH;
		if (fabsf(current - target) < 0.4f)
			current = target;
	};

	step(m_Roll.score, (float)g_GameData.score);
	step(m_Roll.hiScore, (float)hiTarget);
	step(m_Roll.graze, (float)g_GameData.grazeCount);
	step(m_Roll.playTimeSec, (float)g_GameData.playTimeSec);
	step(m_Roll.lives, (float)g_GameData.livesLeft);
	step(m_Roll.maxKillChain, (float)g_GameData.runStats.maxKillChain);
	step(m_Roll.fever, (float)g_GameData.runStats.feverActivations);
	step(m_Roll.bombs, (float)g_GameData.runStats.bombsUsed);
	step(m_Roll.enemies, (float)g_GameData.runStats.enemiesDestroyed);

	if (BOSS_RUSH_MODE)
	{
		int phase = g_GameData.runStats.bossMaxPhaseReached + 1;
		if (phase < 1) phase = 1;
		if (phase > BOSS_RUSH_PHASE_COUNT) phase = BOSS_RUSH_PHASE_COUNT;
		step(m_Roll.phase, (float)phase);
	}

	if (g_GameData.scoreMultiplier > 1.01f)
		step(m_Roll.scoreMul100, g_GameData.scoreMultiplier * 100.0f);

	if (g_GameData.spellBonusCollected > 0)
		step(m_Roll.spellBonus, (float)g_GameData.spellBonusCollected);
}

SceneType ResultScene::Update()
{
	m_AnimTimer++;
	UpdateRollAnimations();

	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_UP) || KeyHelper::IsMenuMoveTrigger(KEY_INPUT_LEFT))
	{
		m_MenuCursor--;
		if (m_MenuCursor < 0) m_MenuCursor = 1;
	}
	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_DOWN) || KeyHelper::IsMenuMoveTrigger(KEY_INPUT_RIGHT))
	{
		m_MenuCursor++;
		if (m_MenuCursor > 1) m_MenuCursor = 0;
	}

	if (KeyHelper::IsTrigger(KEY_INPUT_Z))
		m_MenuCursor = 0;

	if (KeyHelper::IsConfirmTrigger())
	{
		if (m_MenuCursor == 0)
			return SceneType::Game;
		return SceneType::Title;
	}

	if (KeyHelper::IsCancelTrigger())
		return SceneType::Exit;

	return SceneType::None;
}

void ResultScene::DrawBackground() const
{
	const bool clear = g_GameData.isClear;
	const float pulse = 0.5f + 0.5f * sinf((float)m_AnimTimer * 0.06f);

	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / (float)SCREEN_HEIGHT;
		int r, g, b;
		if (clear)
		{
			r = (int)(10 + 28 * (1.0f - t) + pulse * 8.0f);
			g = (int)(8 + 18 * (1.0f - t));
			b = (int)(40 + 35 * (1.0f - t));
		}
		else
		{
			r = (int)(28 + 18 * (1.0f - t));
			g = (int)(6 + 8 * (1.0f - t));
			b = (int)(12 + 10 * (1.0f - t));
		}
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(r, g, b));
	}

	for (int i = 0; i < 48; i++)
	{
		int sx = (i * 149 + m_AnimTimer * 3) % SCREEN_WIDTH;
		int sy = (i * 67 + (i & 1 ? m_AnimTimer : 0)) % SCREEN_HEIGHT;
		int br = 70 + (i * 23) % 160;
		if (((m_AnimTimer + i * 5) % 50) < 5)
			br = 220;
		DrawPixel(sx, sy, GetColor(br, br, br + (clear ? 30 : 0)));
	}

	if (clear)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 + pulse * 50.0f));
		for (int p = 0; p < 12; p++)
		{
			int px = (p * 211 + m_AnimTimer * 6) % SCREEN_WIDTH;
			int py = (m_AnimTimer * 4 + p * 37) % (SCREEN_HEIGHT + 40) - 20;
			unsigned int pc = (p & 1) ? GetColor(255, 220, 80) : GetColor(255, 180, 255);
			DrawCircle(px, py, 2 + (p & 2), pc, TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void ResultScene::DrawRankBadge(int panelLeft, int panelTop, int panelBottom) const
{
	const char rankStr[2] = { g_GameData.rankLetter, '\0' };
	const float pulse = 0.85f + 0.15f * sinf((float)m_AnimTimer * 0.08f);

	unsigned int ringCol = GetColor(0, 140, 180);
	unsigned int rankCol = GetColor(255, 255, 255);
	if (g_GameData.rankLetter == 'S')
	{
		ringCol = GetColor(255, 200, 0);
		rankCol = GetColor(255, 230, 80);
	}
	else if (g_GameData.rankLetter == 'A')
	{
		ringCol = GetColor(80, 220, 140);
		rankCol = GetColor(200, 255, 200);
	}
	else if (g_GameData.rankLetter == 'B')
		ringCol = GetColor(0, 180, 255);

	const int badgeCx = panelLeft + 150;
	const int badgeCy = (panelTop + panelBottom) / 2;
	const int radius = 64;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(60 * pulse));
	DrawCircle(badgeCx, badgeCy, radius + 8, ringCol, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawCircle(badgeCx, badgeCy, radius, GetColor(12, 18, 40), TRUE);
	DrawCircle(badgeCx, badgeCy, radius, ringCol, FALSE);

	SetFontSize(20);
	DrawTextUtf8Centered(badgeCx, badgeCy - 48, GetColor(180, 200, 230), "RANK");

	SetFontSize(80);
	DrawTextUtf8Centered(badgeCx, badgeCy - 36, rankCol, rankStr);

	if (m_IsNewRecord)
	{
		SetFontSize(16);
		const int blink = ((m_AnimTimer / 20) % 2 == 0) ? 255 : 180;
		DrawTextUtf8Centered(badgeCx, badgeCy + 46, GetColor(255, blink, 80), "NEW RECORD!");
	}
}

int ResultScene::CountStatLines() const
{
	int n = 14;
	if (BOSS_RUSH_MODE) n++;
	if (g_GameData.scoreMultiplier > 1.01f) n++;
	if (g_GameData.spellBonusCollected > 0) n++;
	return n;
}

void ResultScene::DrawStatsPanel(int left, int top, int right, int bottom, int lineH) const
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 55);
	DrawBox(left, top, right, bottom, GetColor(8, 12, 32), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(left, top, right, bottom, GetColor(0, 120, 160), FALSE);

	const char* diffLabels[] = { "ＥＡＳＹ", "ＮＯＲＭＡＬ", "ＨＡＲＤ", "ＬＵＮＡＴＩＣ" };
	int dIdx = (int)g_GameData.difficulty;
	if (dIdx < 0 || dIdx >= DifficultyCount()) dIdx = 1;

	const char* modeLabel = "ストーリー";
	switch (g_Session.playMode)
	{
	case PlayMode::Practice:    modeLabel = "練習モード"; break;
	case PlayMode::Tutorial:    modeLabel = "チュートリアル"; break;
	case PlayMode::ScoreAttack: modeLabel = "スコアアタック"; break;
	default:
		modeLabel = BOSS_RUSH_MODE ? "ボスラッシュ" : "ストーリー";
		break;
	}

	const char* shotLabels[] = { "拡散弾", "貫通弾", "誘導弾" };
	int shotIdx = (int)g_Session.shotType;
	if (shotIdx < 0 || shotIdx > 2) shotIdx = 0;

	const int statX = left + 300;
	int y = top + 12;

	auto nextLine = [&](int extraH = 0)
	{
		y += lineH + extraH;
	};

	SetFontSize(20);
	DrawFormatString(statX, y, GetColor(160, 180, 210), "プレイモード  %s", modeLabel);
	nextLine();
	DrawFormatString(statX, y, GetColor(160, 180, 210), "ショット      %s", shotLabels[shotIdx]);
	nextLine(2);

	DrawLine(statX, y, right - 24, y, GetColor(0, 90, 120));
	nextLine(2);

	const int displayScore = (int)(m_Roll.score + 0.5f);
	const int displayHi = (int)(m_Roll.hiScore + 0.5f);
	const int displayGraze = (int)(m_Roll.graze + 0.5f);
	const int displaySec = (int)(m_Roll.playTimeSec + 0.5f);
	const int displayMin = displaySec / 60;
	const int displaySecRem = displaySec % 60;

	SetFontSize(24);
	DrawFormatString(statX, y, GetColor(0, 255, 255), "スコア  %08d", displayScore);
	nextLine(4);

	SetFontSize(20);
	DrawFormatString(statX, y, GetColor(200, 200, 250), "難易度        %s", diffLabels[dIdx]);
	nextLine();
	DrawFormatString(statX, y, GetColor(255, 200, 100), "グレイズ      %d 回", displayGraze);
	nextLine();
	DrawFormatString(statX, y, GetColor(200, 200, 200), "プレイ時間    %02d:%02d", displayMin, displaySecRem);
	nextLine();
	DrawFormatString(statX, y, GetColor(200, 220, 255), "残機          %d", (int)(m_Roll.lives + 0.5f));
	nextLine(2);

	DrawFormatString(statX, y, GetColor(140, 170, 200), "── 戦闘記録 ──");
	nextLine();

	DrawFormatString(statX, y, GetColor(0, 255, 200), "最大連続撃破  %d", (int)(m_Roll.maxKillChain + 0.5f));
	nextLine();
	DrawFormatString(statX, y, GetColor(255, 180, 220), "フィーバー    %d 回", (int)(m_Roll.fever + 0.5f));
	nextLine();
	DrawFormatString(statX, y, GetColor(180, 220, 255), "ボム使用      %d 回", (int)(m_Roll.bombs + 0.5f));
	nextLine();
	DrawFormatString(statX, y, GetColor(200, 255, 180), "撃破数        %d", (int)(m_Roll.enemies + 0.5f));
	nextLine();

	if (BOSS_RUSH_MODE)
	{
		DrawFormatString(statX, y, GetColor(255, 200, 140), "到達形態      第%d形態", (int)(m_Roll.phase + 0.5f));
		nextLine();
	}

	if (g_GameData.scoreMultiplier > 1.01f)
	{
		DrawFormatString(statX, y, GetColor(255, 180, 255), "ボーナス倍率  x%.2f",
			m_Roll.scoreMul100 / 100.0f);
		nextLine();
	}
	if (g_GameData.spellBonusCollected > 0)
	{
		DrawFormatString(statX, y, GetColor(255, 220, 160), "スペル拾得    %d", (int)(m_Roll.spellBonus + 0.5f));
		nextLine();
	}

	const char* hiLabel = (g_Session.playMode == PlayMode::ScoreAttack) ? "SCベスト" : "ハイスコア";
	DrawFormatString(statX, bottom - lineH - 8, GetColor(255, 255, 150),
		"%s(%s)  %08d", hiLabel, diffLabels[dIdx], displayHi);
}

void ResultScene::DrawFooterPanel(const ResultUILayout& L) const
{
	const int cx = SCREEN_WIDTH / 2;
	const int panelL = cx - 480;
	const int panelR = cx + 480;

	if (L.footerPanelBottom <= L.footerPanelTop + 4)
		return;

	if (L.showBonus || L.showAchievements)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 48);
		DrawBox(panelL, L.footerPanelTop, panelR, L.footerPanelBottom, GetColor(6, 10, 28), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawBox(panelL, L.footerPanelTop, panelR, L.footerPanelBottom, GetColor(0, 90, 120), FALSE);
	}

	if (L.showBonus)
	{
		char buf[256];
		buf[0] = '\0';
		if (g_GameData.runStats.noBombUsed)
			strcat_s(buf, "ノーボム　");
		if (g_GameData.runStats.clearedWithoutDamage)
			strcat_s(buf, "無被弾　");
		if (g_GameData.runStats.wavesNoDamage > 0)
		{
			char tmp[48];
			sprintf_s(tmp, "無傷W%d　", g_GameData.runStats.wavesNoDamage);
			strcat_s(buf, tmp);
		}

		SetFontSize(18);
		DrawTextUtf8Centered(cx, L.bonusY, GetColor(255, 230, 140), "クリアボーナス");
		SetFontSize(17);
		DrawTextUtf8Centered(cx, L.bonusY + 22, GetColor(200, 255, 200), buf);
	}

	if (L.showAchievements)
	{
		const int achCount = CountJustUnlocked();
		SetFontSize(18);
		DrawTextUtf8Centered(cx, L.achY, GetColor(255, 215, 0), "新規実績解除");

		SetFontSize(16);
		if (achCount == 1)
		{
			for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT; i++)
			{
				if (!m_JustUnlocked[i]) continue;
				DrawTextUtf8Centered(cx, L.achY + 24, GetColor(0, 255, 180),
					SaveDataGetAchievementName((AchievementId)i));
				break;
			}
		}
		else
		{
			int shown = 0;
			int x0 = cx - (achCount - 1) * 95;
			for (int i = 0; i < (int)AchievementId::ACHIEVEMENT_COUNT && shown < 4; i++)
			{
				if (!m_JustUnlocked[i]) continue;
				DrawTextUtf8Centered(x0 + shown * 190, L.achY + 24,
					GetColor(0, 255, 180), SaveDataGetAchievementName((AchievementId)i));
				shown++;
			}
		}
	}

	SetFontSize(20);
	char titleLine[96];
	sprintf_s(titleLine, "称号　%s", SaveDataGetActiveTitle());
	DrawTextUtf8Centered(cx, L.playerTitleY, GetColor(255, 200, 100), titleLine);
}

void ResultScene::DrawMenuPrompt(int menuBoxTop) const
{
	const int cx = SCREEN_WIDTH / 2;
	const int y0 = menuBoxTop;
	const char* items[] = { "もう一度", "タイトルへ" };

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(cx - 280, y0, cx + 280, y0 + 62, GetColor(8, 12, 28), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(cx - 280, y0, cx + 280, y0 + 62, GetColor(0, 100, 140), FALSE);

	SetFontSize(26);
	for (int i = 0; i < 2; i++)
	{
		const bool sel = (m_MenuCursor == i);
		const int itemX = (i == 0) ? cx - 160 : cx + 40;
		if (sel)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
			DrawBox(itemX - 12, y0 + 10, itemX + 130, y0 + 44, GetColor(0, 60, 90), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		unsigned int col = sel ? GetColor(0, 255, 200) : GetColor(160, 170, 190);
		DrawTextUtf8(itemX, y0 + 16, col, items[i]);
	}

	SetFontSize(15);
	DrawTextUtf8Centered(cx, SCREEN_HEIGHT - 24, GetColor(140, 150, 170),
		"←→選択  Z:もう一度  Enter決定  ESC:終了");
}

void ResultScene::Draw()
{
	const int cx = SCREEN_WIDTH / 2;

	ResultUILayout layout = {};
	ComputeLayout(layout);

	DrawBackground();

	const char* titleStr = g_GameData.isClear ? "ミッションクリア！" : "ミッション失敗…";
	unsigned int titleColor = g_GameData.isClear ? GetColor(255, 215, 0) : GetColor(255, 80, 80);

	SetFontSize(40);
	DrawTextUtf8Centered(cx, layout.headerTitleY, titleColor, titleStr);

	const int titleW = GetTextDrawWidth(titleStr);
	DrawLine(cx - titleW / 2 - 16, layout.sepY, cx + titleW / 2 + 16, layout.sepY,
		g_GameData.isClear ? GetColor(255, 200, 80) : GetColor(180, 60, 60));

	const int panelL = cx - 480;
	const int panelR = cx + 480;
	const int mainH = layout.mainPanelBottom - layout.mainPanelTop;
	const int lineCount = CountStatLines();
	int lineH = mainH / lineCount;
	if (lineH > 24) lineH = 24;
	if (lineH < 19) lineH = 19;

	DrawStatsPanel(panelL, layout.mainPanelTop, panelR, layout.mainPanelBottom, lineH);
	DrawRankBadge(panelL, layout.mainPanelTop, layout.mainPanelBottom);

	DrawFooterPanel(layout);
	DrawMenuPrompt(layout.menuBoxTop);

	SetFontSize(20);
}
