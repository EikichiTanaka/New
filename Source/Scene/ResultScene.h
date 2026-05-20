#pragma once
#include "Scene/SceneBase.h"

class ResultScene : public SceneBase
{
public:
	void Init() override;
	void Update() override;
	void Draw() override;
	void Final() override;

private:
	int m_TargetScore = 0;
	float m_DisplayScore = 0.0f;
	int m_AnimTimer = 0;
	int m_ShakeTimer = 0;
	char m_Rank = 'C';
};
