#pragma once
#include "GameConfig.h"
#include "Game/BulletManager.h"

class Player
{
public:
	void Init(bool practiceMode, bool scoreAttackMode = false);

	void Update(BulletManager& bullets);
	void Draw();

	void OnHit();
	void AddGraze();
	void OnEnemyKill();
	void OnBossHit();
	void AddScore(int amount);
	bool TriggerBomb();

	float GetX() const { return m_X; }
	float GetZ() const { return m_Z; }
	float GetY() const { return PLAYER_Y; }
	float GetRadius() const { return PLAYER_COLLISION_RADIUS; }
	bool IsInvincible() const { return m_InvTimer > 0; }
	bool IsAlive() const { return m_Alive; }
	bool IsPracticeMode() const { return m_PracticeMode; }
	bool IsSlowMode() const;
	bool IsScoreAttackMode() const { return m_ScoreAttackMode; }
	void ForceInvincible(int frames);

	int GetScore() const { return m_Score; }
	int GetLives() const { return m_Lives; }
	int GetBombCount() const { return m_BombCount; }
	int GetGrazeCount() const { return m_GrazeCount; }
	float GetFeverGauge() const { return m_FeverGauge; }
	int GetKillChain() const { return m_KillChain; }
	int GetMaxKillChain() const { return m_MaxKillChain; }
	int GetFeverActivations() const { return m_FeverActivations; }
	int GetBombsUsed() const { return m_BombsUsed; }
	bool IsFeverMode() const { return m_FeverTimer > 0; }
	int GetFeverTimer() const { return m_FeverTimer; }
	static int GetFeverDurationMax() { return FEVER_DURATION_FRAMES; }

	bool ConsumeFeverStartFlash();
	bool ConsumeFeverReadyPing();
	bool IsFeverGaugeReady() const { return m_FeverGauge >= FEVER_GAUGE_READY && m_FeverTimer == 0; }

	void SetLives(int lives) { m_Lives = lives; }
	void SetBombCount(int bombs) { m_BombCount = bombs; }
	void CycleShotType();
	int GetChargeFrames() const { return m_ChargeFrames; }

private:
	void FireShots(BulletManager& bullets, bool fever);
	void FireChargeShot(BulletManager& bullets);
	void AddFeverGauge(float amount);
	void TryTriggerFever();

	float m_X, m_Z;
	int m_FireCooldown;
	int m_InvTimer;
	bool m_Alive;
	bool m_PracticeMode;

	int m_Score;
	int m_Lives;
	int m_BombCount;
	int m_GrazeCount;
	float m_FeverGauge;
	int m_FeverTimer;
	int m_KillChain;
	int m_KillChainTimer;
	int m_GrazeRushCount;
	int m_GrazeRushTimer;
	int m_MaxKillChain;
	int m_FeverActivations;
	int m_BombsUsed;
	bool m_FeverStartFlash;
	bool m_FeverReadyPing;
	int m_ChargeFrames;
	bool m_WasShootHeld;
	bool m_ScoreAttackMode;
};
