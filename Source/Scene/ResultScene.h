#pragma once

#include "Scene/SceneBase.h"
#include "Common/SaveData.h"

struct ResultUILayout
{
	int headerTitleY;
	int sepY;
	int mainPanelTop;
	int mainPanelBottom;
	int footerPanelTop;
	int footerPanelBottom;
	bool showBonus;
	bool showAchievements;
	int bonusY;
	int achY;
	int playerTitleY;
	int menuBoxTop;
};

class ResultScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	void FinalizeAchievements();
	int CountJustUnlocked() const;
	int CountStatLines() const;
	bool HasClearBonus() const;
	void ComputeLayout(ResultUILayout& layout) const;
	void DrawBackground() const;
	void DrawRankBadge(int panelLeft, int panelTop, int panelBottom) const;
	void DrawStatsPanel(int left, int top, int right, int bottom, int lineH) const;
	void DrawFooterPanel(const ResultUILayout& layout) const;
	void DrawMenuPrompt(int menuBoxTop) const;
	void UpdateRollAnimations();

	struct ResultRollAnim
	{
		float score = 0.0f;
		float hiScore = 0.0f;
		float graze = 0.0f;
		float playTimeSec = 0.0f;
		float lives = 0.0f;
		float maxKillChain = 0.0f;
		float fever = 0.0f;
		float bombs = 0.0f;
		float enemies = 0.0f;
		float phase = 0.0f;
		float spellBonus = 0.0f;
		float scoreMul100 = 0.0f;
	};

	int m_AnimTimer;
	ResultRollAnim m_Roll;
	int m_MenuCursor;
	bool m_IsNewRecord;
	bool m_JustUnlocked[(int)AchievementId::ACHIEVEMENT_COUNT];
};
