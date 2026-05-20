#pragma once
#include "Scene/SceneBase.h"
#include "Game/GamePlay.h"

class GameScene : public SceneBase
{
public:
	void Init() override;
	void Update() override;
	void Draw() override;
	void Final() override;
private:
	GamePlay m_Play;
};
