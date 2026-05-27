#pragma once
#include "Game/Enemy.h"
#include "GameConfig.h"
#include "Game/BulletManager.h"
#include "Common/GameSession.h"

class EnemyManager
{
public:
	void Init(Difficulty diff);
	void ConfigureStage(StageStart stage, PlayMode mode, int stageChapter = 0,
		int bossRushStartPhase = 0);

	void Update(float playerX, float playerZ, BulletManager& bullets);
	void Draw() const;

	void SpawnEnemy(EnemyType type, float x, float z);
	void DamageAllEnemies(int dmg, BulletManager& bullets);

	Enemy* GetEnemies() { return m_Enemies; }
	const Enemy* GetEnemies() const { return m_Enemies; }
	int GetEnemyMax() const { return ENEMY_MAX; }

	bool IsBossActive() const { return m_BossActive; }
	bool IsMidBossActive() const { return m_MidBossActive; }
	bool IsSpellBreakActive() const { return m_SpellBreakTimer > 0; }
	int GetSpellBreakTimer() const { return m_SpellBreakTimer; }

	float GetBossX() const { return m_BossX; }
	float GetBossY() const { return m_BossY; }
	float GetBossZ() const { return m_BossZ; }
	float GetBossRadius() const { return m_BossHitRadius; }
	float GetBossDrawRadius() const { return m_BossDrawRadius; }
	int GetBossHp() const { return m_BossHp; }
	int GetBossMaxHp() const { return m_BossMaxHp; }
	int GetBossPhase() const { return m_BossPhase; }
	int GetBossTimer() const { return m_BossTimer; }

	float GetMidBossX() const { return m_MidBossX; }
	float GetMidBossZ() const { return m_MidBossZ; }
	float GetMidBossRadius() const { return m_MidBossHitRadius; }
	float GetMidBossDrawRadius() const { return m_MidBossRadius; }

	void TakeBossDamage(int dmg, BulletManager& bullets);
	bool TryApplyBossBulletHit(int dmg, BulletManager& bullets);
	bool TryApplyMidBossBulletHit(int dmg, BulletManager& bullets);

	int GetCurrentWave() const { return m_WaveIndex + 1; }
	bool IsAllWavesComplete() const { return m_AllWavesComplete; }
	int GetStageChapter() const { return m_StageChapter; }

	bool IsBossIntroActive() const { return m_BossIntroTimer > 0; }
	int GetBossIntroTimer() const { return m_BossIntroTimer; }

	void SpawnSplitterChildren(float x, float z);

private:
	Enemy m_Enemies[ENEMY_MAX];
	Difficulty m_Difficulty;
	PlayMode m_PlayMode;
	bool m_EnableMidBoss1;
	bool m_EnableMidBoss2;

	int m_WaveIndex;
	int m_WaveTimer;
	int m_SpawnTimer;
	int m_SpawnCount;
	bool m_AllWavesComplete;
	int m_WaveDanmakuTimer;

	bool m_BossActive;
	float m_BossX, m_BossY, m_BossZ;
	float m_BossDrawRadius;
	float m_BossHitRadius;
	int m_BossHp;
	int m_BossMaxHp;
	int m_BossTimer;
	int m_BossPatternTimer;
	int m_BossPhase;
	int m_SpellBreakTimer;
	int m_BossHitCooldown;
	int m_MidBossHitCooldown;

	bool m_MidBossActive;
	int m_MidBossId;
	float m_MidBossX, m_MidBossZ;
	float m_MidBossRadius;
	float m_MidBossHitRadius;
	int m_MidBossHp;
	int m_MidBossMaxHp;
	int m_MidBossTimer;

	int m_StageChapter;
	int m_BossRushStartPhase;
	int m_BossIntroTimer;
	bool m_BossIntroClearedBullets;

	void SpawnBoss();
	void SpawnMidBoss(int id);
	void UpdateBossMovement();
	void UpdateBoss(float playerX, float playerZ, BulletManager& bullets);
	void UpdateMidBoss(float playerX, float playerZ, BulletManager& bullets);
	void UpdateWaveDanmaku(float playerX, float playerZ, BulletManager& bullets);
	void OnWaveSectionClear();
	void DrawBoss() const;
	void DrawMidBoss() const;

	float GetBulletSpeed() const;
	int ScaleInterval(int base) const;
};
