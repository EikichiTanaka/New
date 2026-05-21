#pragma once

// ============================================================
// GameScene.h
// ゲーム本編シーン - 3D弾幕シューティング
// ============================================================

#include "Scene/SceneBase.h"
#include "Game/Player.h"
#include "Game/BulletManager.h"
#include "Game/EnemyManager.h"
#include "Game/Effect.h"

class GameScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	void SetupSceneLighting();
	void DisableSceneLighting();
	void AddScreenShake(int frames, float magnitude);
	void SetupCamera();
	void DrawField();
	void CheckCollisions();
	void DrawHud();
	void DrawPauseOverlay();

	struct GameItem
	{
		float x, y, z;
		float vx, vy, vz;
		int type;
		bool active;
	};
	static constexpr int ITEM_MAX = 32;
	GameItem m_Items[ITEM_MAX];

	void InitItems();
	void SpawnItem(float x, float z, int type);
	void UpdateItems();
	void DrawItems() const;
	void ExecuteBomb();

	Player        m_Player;
	BulletManager m_Bullets;
	EnemyManager  m_Enemies;
	Effect        m_Effect;

	int  m_FrameCount;
	bool m_GameEnded;
	int  m_ClearDelayTimer;
	int  m_DirLightHandle;
	int  m_PlayerLightHandle;
	int  m_ForwardLightHandle;
	int  m_BossHitFxCooldown;
	float m_CamX, m_CamY, m_CamZ;
	float m_LookX, m_LookY, m_LookZ;
	int   m_ShakeTimer;
	float m_ShakeMag;

	bool m_Paused;
	int  m_HitStopTimer;
	int  m_PrevWave;
	bool m_WaveNoDamage;
	int  m_PrevSpellBreakTimer;

	void DrawBossIntroOverlay() const;
	void UpdateBackgroundBgm();
	void SpawnSpellBreakBonuses();
	void FinalizeRunStats();
};
