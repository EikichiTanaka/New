#pragma once
#include "Scene/SceneBase.h"

class OptionsScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	int m_CursorRow;
	bool m_NeedGraphicsApply;
};
