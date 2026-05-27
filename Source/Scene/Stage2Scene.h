#pragma once

// ============================================================
// Stage2Scene.h
// Stage 2: TPS-style 3D boss fight (boss-only, like Stage 1
// in Boss Rush mode but in full 3D space).
//   - Camera: third-person, follows player from behind/above.
//   - Mouse: yaw/pitch to aim. Player ship rotates with view.
//   - WASD + E/Q: full 6DOF movement (forward/back/strafe/up/down).
//   - Space (or Z): auto-fire bullets along look direction.
//   - X / B: bomb (clears bullets, damages boss).
//   - Boss has 4 phases with HP / spell break / different
//     omnidirectional attack patterns.
//   - Rendering kept lightweight (low sphere segments, few stars,
//     small particle counts).
// ============================================================

#include "Scene/SceneBase.h"
#include "Game/BulletManager.h"
#include "Game/Effect.h"
#include "GameConfig.h"

class Stage2Scene : public SceneBase
{
public:
	~Stage2Scene() override;
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	static constexpr int STAGE2_BOSS_PHASES = BOSS_RUSH_PHASE_COUNT;

	void SetupCamera();
	void ComputeForward(float& fx, float& fy, float& fz) const;
	void ComputeRight(float& rx, float& ry, float& rz) const;

	void UpdateMouseLook();
	void UpdatePlayer();
	void HandleShooting();
	void HandleBomb();
	void UpdateScreenShake();

	void UpdateBoss();
	void UpdateBossAttack();
	void StartNextPhase();
	void TakeBossDamage(int dmg);

	void CheckCollisions();
	void OnPlayerHit();

	void SetupSceneLighting();
	void DisableSceneLighting();

	void DrawBackground2D() const;
	void DrawSkybox3D() const;
	void DrawArena3D() const;
	void DrawPlayerShip3D() const;
	void DrawBoss3D() const;
	void DrawCrosshair() const;
	void DrawHud() const;
	void DrawScreenFx() const;
	void DrawSpeedFx() const;
	void DrawPauseOverlay() const;
	void DrawSpellBanner() const;

	SceneType UpdatePauseMenu();
	void ExitToTitle();
	void AddScreenShake(int frames, float magnitude);

	void EnterStage();
	void LeaveStage();
	void RecenterMouse();

	BulletManager m_Bullets;
	Effect        m_Effect;

	// Player
	float m_PlayerX, m_PlayerY, m_PlayerZ;
	float m_PlayerVX, m_PlayerVY, m_PlayerVZ;
	float m_PlayerSpeed;
	float m_Yaw;
	float m_Pitch;
	int   m_Lives;
	int   m_BombCount;
	int   m_Score;
	int   m_GrazeCount;
	int   m_KillChain;
	int   m_MaxKillChain;
	int   m_KillChainTimer;
	int   m_FireCooldown;
	int   m_InvTimer;
	int   m_BombsUsed;
	bool  m_Alive;
	bool  m_BombKeyHeld;
	int   m_ComboCount;
	int   m_ComboTimer;
	int   m_BossHitPopTimer;
	int   m_BossHitPopScore;

	// Smoothed TPS camera anchor
	float m_CamAnchorX, m_CamAnchorY, m_CamAnchorZ;

	// Boss
	float m_BossX, m_BossY, m_BossZ;
	float m_BossVX, m_BossVY, m_BossVZ;
	float m_BossTX, m_BossTY, m_BossTZ;  // target point for drift
	float m_BossDrawRadius;
	float m_BossHitRadius;
	int   m_BossHp;
	int   m_BossMaxHp;
	int   m_BossPhase;
	int   m_BossPatternTimer;
	int   m_BossTimer;
	int   m_SpellBreakTimer;
	int   m_BossIntroTimer;
	int   m_BossHitCooldown;
	int   m_BossHitFlash;
	bool  m_BossActive;
	bool  m_BossDefeated;
	bool  m_BossPracticeMode;

	// Mouse capture
	int  m_MouseCenterX, m_MouseCenterY;
	bool m_MouseCaptured;
	bool m_PendingMouseRecenter;

	// Frame / FX state
	int  m_FrameCount;
	int  m_ClearDelayTimer;
	int  m_HitStopTimer;
	int  m_ShakeTimer;
	float m_ShakeMag;
	int  m_DamageFlashTimer;
	int  m_BombFlashTimer;
	int  m_SpellBannerTimer;

	bool m_Paused;
	int  m_PauseMenuCursor;
	int  m_DirLightHandle;
	bool m_LightingActive;
};
