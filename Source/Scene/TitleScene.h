#pragma once

#include "Scene/SceneBase.h"

class TitleScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	int m_AnimTimer;
};
