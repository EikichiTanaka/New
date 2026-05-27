#pragma once
#include "Scene/SceneBase.h"
#include "GameConfig.h"

class DifficultySelectScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	int m_CursorRow;
	int m_ModeIdx = 0;
	int m_ChapterIdx = 0;
	int m_StageIdx = 0;
	int m_DiffIdx = 1;
	int m_PhaseStartIdx = 0;
	int m_ShotIdx = 0;
	// 1=一面（弾幕ボスラッシュ）、2=二面（全方位3Dシューター）
	int m_StageNumberIdx = 0;
	bool m_ViewAchievements;

	void CycleRow(int dir);
	void ApplySelection();
	void ApplyTutorialSelection();
	void DrawAchievementsOverlay();
	void DrawBossRushInfoPanel(int left, int top, int right, int bottom,
		Difficulty diff, bool practiceMode, int startPhase, bool isTutorial) const;
	void DrawPrepMenuRows(int labelX, int valueX, int startY, int rowH, int rowCount,
		const char* const* names, const char* const* values) const;
};
