#pragma once

#include "Scene/SceneBase.h"

class DifficultySelectScene : public SceneBase
{
public:
	void Init() override;
	void Update() override;
	void Draw() override;
	void Final() override;

private:
	int m_SelectedIndex;
	int m_AnimTimer;
	int m_StartDelayTimer;
};
