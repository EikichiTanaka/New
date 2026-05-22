#pragma once
#include "Scene/SceneBase.h"
#include "Game/Player.h"
#include "Game/BulletManager.h"
#include "Game/Effect.h"

class TutorialScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	void SetupCamera();
	void SpawnTutorialBullets();
	void DrawBackground2D() const;
	void DrawHud();
	void DrawStepText();

	Player m_Player;
	BulletManager m_Bullets;
	Effect m_Effect;

	int m_Step;
	int m_StepTimer;
	int m_BulletSpawnTimer;
	float m_CamX, m_CamY, m_CamZ;
	float m_LookX, m_LookY, m_LookZ;
};
