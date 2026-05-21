#pragma once
#include "Scene/SceneBase.h"

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
	int m_ShotIdx = 0;
	bool m_ViewAchievements;

	void CycleRow(int dir);
	void ApplySelection();
	void ApplyTutorialSelection();
	void DrawAchievementsOverlay();
};
