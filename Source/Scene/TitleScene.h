#pragma once
#include "Scene/SceneBase.h"

class TitleScene : public SceneBase
{
public:
	void Init() override;
	void Update() override;
	void Draw() override;
	void Final() override;

private:
	int m_Timer = 0;
	struct RainNode { float x, y, speed; int length; };
	static const int MAX_RAIN = 40;
	RainNode m_Rain[MAX_RAIN] = {};
};
