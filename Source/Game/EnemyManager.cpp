#include "Game/EnemyManager.h"
#include "Game/SilhouetteDraw.h"
#include "Common/ResourceManager.h"
#include "DxLib.h"
#include <cmath>

// --- Init: マネージャーの初期化 ---
void EnemyManager::Init(Difficulty diff)
{
	m_Difficulty = diff;
	m_WaveIndex = 0;
	m_WaveTimer = 90;
	m_WaveDanmakuTimer = 0;
	m_SpawnTimer = 0;
	m_SpawnCount = 0;
	m_AllWavesComplete = false;

	m_BossActive = false;
	m_BossX = 0.0f;
	m_BossY = PLAYER_Y;
	m_BossZ = 400.0f;
	m_BossDrawRadius = BOSS_DRAW_RADIUS_REF;
	m_BossHitRadius = GetBossCollisionRadius(0);
	m_BossHp = 0;
	m_BossMaxHp = 0;
	m_BossTimer = 0;
	m_BossPatternTimer = 0;
	m_BossPhase = 0;
	m_WaveDanmakuTimer = 0;
	m_SpellBreakTimer = 0;
	m_BossHitCooldown = 0;
	m_MidBossHitCooldown = 0;
	m_StageChapter = 0;
	m_BossRushStartPhase = 0;
	m_BossIntroTimer = 0;
	m_BossIntroClearedBullets = false;
	m_MidBossActive = false;
	m_MidBossId = 0;
	m_MidBossHitRadius = MIDBOSS_COLLISION_RADIUS;
	m_MidBossHp = 0;
	m_EnableMidBoss1 = true;
	m_EnableMidBoss2 = true;
	m_PlayMode = PlayMode::Story;

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		m_Enemies[i].Deactivate();
	}
}

void EnemyManager::ConfigureStage(StageStart stage, PlayMode mode, int stageChapter,
	int bossRushStartPhase)
{
	m_PlayMode = mode;
	m_StageChapter = stageChapter;
	if (m_StageChapter < 0) m_StageChapter = 0;
	if (m_StageChapter >= STAGE_CHAPTER_COUNT) m_StageChapter = STAGE_CHAPTER_COUNT - 1;
	m_BossRushStartPhase = ClampBossRushStartPhase(bossRushStartPhase);

	// [BOSS_RUSH_MODE] 面・ウェーブなし。選択形態から即ボス戦
	if (BOSS_RUSH_MODE)
	{
		m_StageChapter = 0;
		m_EnableMidBoss1 = false;
		m_EnableMidBoss2 = false;
		m_WaveIndex = 4;
		m_WaveTimer = 0;
		m_WaveDanmakuTimer = 0;
		SpawnBoss();
		return;
	}

	m_EnableMidBoss1 = (stage == StageStart::All || stage == StageStart::Wave1 || stage == StageStart::Wave2);
	m_EnableMidBoss2 = (stage == StageStart::All);

	int midId = 0;
	if (StageStartIsMidBossOnly(stage, midId))
	{
		m_WaveIndex = 0;
		m_WaveTimer = 30;
		SpawnMidBoss(midId);
		return;
	}
	if (StageStartSkipsToBoss(stage))
	{
		m_WaveIndex = 4;
		m_WaveTimer = 50;
		return;
	}
	m_WaveIndex = StageStartToWaveIndex(stage);
	m_WaveTimer = WAVE_SPAWN_DELAY_FRAMES;
	m_WaveDanmakuTimer = 0;
}

float EnemyManager::GetBulletSpeed() const
{
	return DifficultyBulletSpeed(m_Difficulty);
}

static EnemyType PickStageEnemy(int chapter, int wave, int spawnIndex)
{
	if (chapter >= 1)
	{
		if (wave >= 2 && (spawnIndex % 3) == 0) return EnemyType::Eraser;
		if ((spawnIndex % 4) == 1) return EnemyType::Shield;
		if ((spawnIndex % 5) == 2) return EnemyType::Splitter;
		if ((spawnIndex % 3) == 0) return EnemyType::Sniper;
		return EnemyType::Spinner;
	}
	if ((spawnIndex % 5) == 2) return EnemyType::Spinner;
	if ((spawnIndex % 7) == 4) return EnemyType::Tank;
	return EnemyType::Basic;
}

int EnemyManager::ScaleInterval(int base) const
{
	return (int)(base * DifficultyDanmakuIntervalMul(m_Difficulty));
}

// --- SpawnEnemy: 非アクティブな敵枠を探して生成 ---
void EnemyManager::SpawnEnemy(EnemyType type, float x, float z)
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (!m_Enemies[i].IsActive())
		{
			m_Enemies[i].Init(type, x, z, m_Difficulty);
			break;
		}
	}
}

// --- DamageAllEnemies: 画面上の敵すべてにダメージを与える（ボム用） ---
void EnemyManager::DamageAllEnemies(int dmg, BulletManager& bullets)
{
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (m_Enemies[i].IsActive())
			m_Enemies[i].TakeDamage(dmg);
	}

	if (m_BossActive)
		TakeBossDamage(dmg, bullets);
	if (m_MidBossActive)
	{
		m_MidBossHp -= dmg;
		if (m_MidBossHp <= 0)
		{
			m_MidBossHp = 0;
			m_MidBossActive = false;
			if (m_MidBossId == 1) m_WaveIndex = 1;
			else m_WaveIndex = 3;
			m_WaveTimer = 90;
			m_WaveDanmakuTimer = 0;
			m_SpawnTimer = 0;
			m_SpawnCount = 0;
		}
	}
}

bool EnemyManager::TryApplyBossBulletHit(int dmg, BulletManager& bullets)
{
	if (!m_BossActive || m_SpellBreakTimer > 0 || m_BossHitCooldown > 0)
		return false;

	m_BossHitCooldown = BOSS_HIT_IFRAMES;
	TakeBossDamage(dmg, bullets);
	return true;
}

bool EnemyManager::TryApplyMidBossBulletHit(int dmg, BulletManager& bullets)
{
	(void)bullets;
	if (!m_MidBossActive || m_MidBossHitCooldown > 0)
		return false;

	m_MidBossHitCooldown = MIDBOSS_HIT_IFRAMES;
	m_MidBossHp -= dmg;
	if (m_MidBossHp <= 0)
	{
		m_MidBossHp = 0;
		m_MidBossActive = false;
		if (m_MidBossId == 1) m_WaveIndex = 1;
		else m_WaveIndex = 3;
		m_WaveTimer = 90;
		m_WaveDanmakuTimer = 0;
		m_SpawnTimer = 0;
		m_SpawnCount = 0;
	}
	return true;
}

void EnemyManager::TakeBossDamage(int dmg, BulletManager& bullets)
{
	if (!m_BossActive || m_SpellBreakTimer > 0) return;

	m_BossHp -= dmg;
	if (m_BossHp <= 0)
	{
		m_BossHp = 0;
		m_BossPhase++;
		const int phaseMax = BOSS_RUSH_MODE ? BOSS_RUSH_PHASE_COUNT : 3;
		if (m_BossPhase >= phaseMax)
		{
			m_BossActive = false;
			m_AllWavesComplete = true;
		}
		else
		{
			m_SpellBreakTimer = BOSS_RUSH_MODE
				? GetBossRushSpellBreakFrames(m_Difficulty)
				: SPELL_BREAK_FRAMES;
			m_BossPatternTimer = 0;
			bullets.ClearEnemyBullets();
			if (BOSS_RUSH_MODE)
			{
				m_BossHp = GetBossRushPhaseHp(m_Difficulty, m_BossPhase,
					m_PlayMode == PlayMode::Practice);
				m_BossMaxHp = m_BossHp;
				m_BossHitRadius = GetBossCollisionRadius(m_BossPhase);
			}
			else if (m_BossPhase == 1)
			{
				m_BossHp = 130;
				m_BossMaxHp = 130;
			}
			else if (m_BossPhase == 2)
			{
				m_BossHp = 180;
				m_BossMaxHp = 180;
			}
		}
	}
}

void EnemyManager::SpawnMidBoss(int id)
{
	m_MidBossActive = true;
	m_MidBossId = id;
	m_MidBossX = 0.0f;
	m_MidBossZ = 380.0f;
	m_MidBossRadius = MIDBOSS_DRAW_RADIUS_REF;
	m_MidBossHitRadius = MIDBOSS_COLLISION_RADIUS;
	m_MidBossTimer = 0;
	if (id == 1)
	{
		m_MidBossHp = MIDBOSS_HP_1;
		m_MidBossMaxHp = MIDBOSS_HP_1;
	}
	else
	{
		m_MidBossHp = MIDBOSS_HP_2;
		m_MidBossMaxHp = MIDBOSS_HP_2;
	}
}

void EnemyManager::UpdateMidBoss(float playerX, float playerZ, BulletManager& bullets)
{
	if (!m_MidBossActive) return;
	m_MidBossTimer++;
	m_MidBossX = sinf((float)m_MidBossTimer / 30.0f) * 140.0f;

	float spd = GetBulletSpeed();
	float radMul = DifficultyBulletRadiusMul(m_Difficulty);

	if (m_MidBossId == 1)
	{
		if (m_MidBossTimer % ScaleInterval(26) == 0)
		{
			for (int i = 0; i < 22; i++)
			{
				float a = (i * 2.0f * 3.14159265f) / 22.0f + m_MidBossTimer * 0.05f;
				bullets.AddEnemyBullet(m_MidBossX, PLAYER_Y, m_MidBossZ,
					cosf(a) * spd, 0.0f, sinf(a) * spd,
					EBULLET_RADIUS * radMul, GetColor(255, 120, 200));
			}
		}
	}
	else
	{
		if (m_MidBossTimer % ScaleInterval(14) == 0)
		{
			float a1 = m_MidBossTimer * 0.12f;
			bullets.AddEnemyBullet(m_MidBossX, PLAYER_Y, m_MidBossZ, cosf(a1) * spd, 0.0f, sinf(a1) * spd,
				EBULLET_RADIUS * radMul, GetColor(255, 200, 80));
			bullets.AddEnemyBullet(m_MidBossX, PLAYER_Y, m_MidBossZ, cosf(a1 + 3.14f) * spd, 0.0f, sinf(a1 + 3.14f) * spd,
				EBULLET_RADIUS * radMul, GetColor(255, 200, 80));
		}
		if (m_MidBossTimer % ScaleInterval(40) == 0)
		{
			for (int i = 0; i < 30; i++)
			{
				float a = (i * 2.0f * 3.14159265f) / 30.0f;
				bullets.AddEnemyBullet(m_MidBossX, PLAYER_Y, m_MidBossZ,
					cosf(a) * spd * 0.85f, 0.0f, sinf(a) * spd * 0.85f,
					EBULLET_RADIUS * radMul * 1.05f, GetColor(200, 80, 255));
			}
		}
	}
}

void EnemyManager::OnWaveSectionClear()
{
	if (m_WaveIndex == 0 && m_EnableMidBoss1 && m_PlayMode == PlayMode::Story)
	{
		SpawnMidBoss(1);
		return;
	}
	if (m_WaveIndex == 2 && m_EnableMidBoss2 && m_PlayMode == PlayMode::Story)
	{
		SpawnMidBoss(2);
		return;
	}
	m_WaveIndex++;
	m_WaveTimer = 80;
	m_WaveDanmakuTimer = 0;
	m_SpawnTimer = 0;
	m_SpawnCount = 0;
}

// --- SpawnBoss: ボス出現（BOSS_RUSH_MODE では開幕） ---
void EnemyManager::SpawnBoss()
{
	m_BossActive = true;
	m_BossX = 0.0f;
	m_BossY = PLAYER_Y;
	m_BossZ = 400.0f;
	m_BossDrawRadius = BOSS_DRAW_RADIUS_REF;

	const int startPhase = BOSS_RUSH_MODE ? m_BossRushStartPhase : 0;
	m_BossHitRadius = GetBossCollisionRadius(startPhase);

	if (BOSS_RUSH_MODE)
		m_BossHp = GetBossRushPhaseHp(m_Difficulty, startPhase, m_PlayMode == PlayMode::Practice);
	else
	{
		m_BossHp = (m_StageChapter >= 1) ? 110 : 90;
		m_BossHitRadius = 38.0f;
	}

	m_BossMaxHp = m_BossHp;
	m_BossPhase = startPhase;
	m_BossTimer = 0;
	m_BossPatternTimer = 0;
	m_BossIntroTimer = (BOSS_RUSH_MODE && startPhase > 0) ? 60 : BOSS_INTRO_FRAMES;
	m_BossIntroClearedBullets = false;
}

void EnemyManager::SpawnSplitterChildren(float x, float z)
{
	SpawnEnemy(EnemyType::Basic, x - 40.0f, z + 20.0f);
	SpawnEnemy(EnemyType::Basic, x + 40.0f, z + 20.0f);
}

// --- UpdateWaveDanmaku: ウェーブ背景弾幕（弾幕カグラ級の密度） ---
void EnemyManager::UpdateWaveDanmaku(float playerX, float playerZ, BulletManager& bullets)
{
	if (m_BossActive || m_AllWavesComplete || m_WaveTimer > 0) return;

	m_WaveDanmakuTimer++;
	if (m_WaveDanmakuTimer < WAVE_DANMAKU_WARMUP_FRAMES)
		return;
	if ((m_WaveDanmakuTimer % WAVE_DANMAKU_FRAME_INTERVAL) != 0)
		return;

	float spd = GetBulletSpeed();

	const float pi = 3.14159265f;
	float emitX = sinf((float)m_WaveDanmakuTimer * 0.045f) * 220.0f;
	float emitZ = 280.0f + (float)m_WaveIndex * 90.0f;
	float smallR = EBULLET_RADIUS * 0.82f;

	auto ringBurst = [&](int count, float speedMul, unsigned int col, float spin)
	{
		for (int i = 0; i < count; i++)
		{
			float a = (i * 2.0f * pi) / count + spin;
			bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ,
				cosf(a) * spd * speedMul, 0.0f, sinf(a) * spd * speedMul,
				smallR, col);
		}
	};

	switch (m_WaveIndex)
	{
	case 0:
		if (m_WaveDanmakuTimer % 48 == 0)
			ringBurst(8, 0.95f, GetColor(255, 80, 180), (float)m_WaveDanmakuTimer * 0.035f);
		if (m_WaveDanmakuTimer % 18 == 0)
		{
			const int side = (m_WaveDanmakuTimer / 18) % 2 == 0 ? -1 : 1;
			bullets.AddEnemyBullet(side * 320.0f, PLAYER_Y, FIELD_HALF_D - 50.0f,
				side * 0.25f, 0.0f, -spd, smallR, GetColor(0, 220, 255));
		}
		break;

	case 1:
		if (m_WaveDanmakuTimer % 36 == 0)
			ringBurst(12, 0.88f, GetColor(255, 200, 60), (float)m_WaveDanmakuTimer * 0.06f);
		if (m_WaveDanmakuTimer % 48 == 0)
		{
			float dx = playerX - emitX;
			float dz = playerZ - emitZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				for (int n = -1; n <= 1; n++)
				{
					float off = (float)n * 0.12f;
					float vx = ndx * cosf(off) - ndz * sinf(off);
					float vz = ndx * sinf(off) + ndz * cosf(off);
					bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ,
						vx * spd * 1.1f, 0.0f, vz * spd * 1.1f,
						EBULLET_RADIUS, GetColor(255, 60, 80));
				}
			}
		}
		break;

	case 2:
		if (m_WaveDanmakuTimer % 18 == 0)
		{
			float a1 = (float)m_WaveDanmakuTimer * 0.14f;
			float a2 = a1 + pi;
			bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ, cosf(a1) * spd, 0.0f, sinf(a1) * spd, smallR, GetColor(255, 120, 255));
			bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ, cosf(a2) * spd, 0.0f, sinf(a2) * spd, smallR, GetColor(255, 120, 255));
		}
		if (m_WaveDanmakuTimer % 60 == 0)
			ringBurst(14, 0.75f, GetColor(255, 0, 120), (float)m_WaveDanmakuTimer * 0.02f);
		break;

	case 3:
		if (m_WaveDanmakuTimer % 14 == 0)
		{
			float rx = (float)(GetRand(700) - 350);
			bullets.AddEnemyBullet(rx, PLAYER_Y, FIELD_HALF_D + 30.0f,
				0.0f, 0.0f, -spd * 1.15f, smallR * 0.9f, GetColor(0, 255, 220));
		}
		if (m_WaveDanmakuTimer % 42 == 0)
			ringBurst(14, 0.82f, GetColor(255, 50, 255), (float)m_WaveDanmakuTimer * 0.045f);
		if (m_WaveDanmakuTimer % 22 == 0)
		{
			for (int lane = -2; lane <= 2; lane++)
			{
				bullets.AddEnemyBullet(lane * 90.0f, PLAYER_Y, 420.0f,
					(float)lane * 0.25f, 0.0f, -spd * 0.9f, smallR, GetColor(255, 180, 0));
			}
		}
		break;

	default:
		break;
	}

	if (m_StageChapter >= 1 && m_WaveDanmakuTimer % 36 == 0)
	{
		float twist = (float)m_WaveDanmakuTimer * 0.08f;
		float lane = (float)((GetRand(1000) % 7) - 3) * 70.0f;
		bullets.AddEnemyBullet(lane, PLAYER_Y, FIELD_HALF_D - 20.0f,
			sinf(twist) * spd * 0.6f, 0.0f, -spd * 0.95f,
			smallR * 0.9f, GetColor(180, 80, 255));
	}
}

// --- UpdateBossMovement: ボス位置（形態切替・スペル破壊中も継続） ---
void EnemyManager::UpdateBossMovement()
{
	if (!m_BossActive || m_BossIntroTimer > 0)
		return;

	m_BossTimer++;
	m_BossX = sinf((float)m_BossTimer / 45.0f) * 160.0f;
	m_BossZ = 350.0f + cosf((float)m_BossTimer / 60.0f) * 35.0f;
}

// --- UpdateBoss: スペルカード攻撃パターン ---
void EnemyManager::UpdateBoss(float playerX, float playerZ, BulletManager& bullets)
{
	if (!m_BossActive || m_BossIntroTimer > 0) return;

	m_BossPatternTimer++;

	const bool rush = BOSS_RUSH_MODE;
	const BossRushDifficultyTune& tune = GetBossRushTune(m_Difficulty);

	int fireIntervalScale = 1;
	if (!rush)
	{
		if (m_Difficulty == Difficulty::Easy)   fireIntervalScale = 2;
		if (m_Difficulty == Difficulty::Hard)   fireIntervalScale = 0;
	}

	float bulletSpd = GetBulletSpeed();
	float bulletRad = EBULLET_RADIUS;
	if (rush)
	{
		bulletSpd *= tune.bulletSpeedMul;
		bulletRad *= tune.bulletRadiusMul;
	}
	else
	{
		bulletRad *= DifficultyBulletRadiusMul(m_Difficulty);
	}

	auto bossInterval = [&](int rushBase, int legacyBase, int legacyMul) -> int
	{
		if (rush)
			return BossRushScaleFireInterval(rushBase, m_Difficulty);
		return legacyBase + fireIntervalScale * legacyMul;
	};

	auto bossCount = [&](int rushBase, int legacyBase) -> int
	{
		if (rush)
			return BossRushScaleBulletCount(rushBase, m_Difficulty);
		if (m_Difficulty == Difficulty::Easy)  return (int)((float)legacyBase * 0.82f);
		if (m_Difficulty == Difficulty::Hard)  return (int)((float)legacyBase * 1.15f);
		if (m_Difficulty == Difficulty::Lunatic) return (int)((float)legacyBase * 1.22f);
		return legacyBase;
	};

	// --- 段階別攻撃パターン（スペルカード） ---
	if (m_BossPhase == 0)
	{
		const int interval = bossInterval(16, 22, 6);
		if (m_BossPatternTimer % interval == 0)
		{
			const int count = bossCount(30, 24);
			const float offsetAngle = (m_BossPatternTimer * 0.055f);
			for (int i = 0; i < count; i++)
			{
				const float angle = (i * 2.0f * 3.14159265f) / count + offsetAngle;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * bulletSpd * 0.9f, 0.0f, sinf(angle) * bulletSpd * 0.9f,
					bulletRad * 1.05f, GetColor(255, 50, 150));
			}
		}
		const int aimedInterval = rush ? bossInterval(10, 14, 0) : 14;
		if (m_BossPatternTimer % aimedInterval == 0)
		{
			float dx = playerX - m_BossX;
			float dz = playerZ - m_BossZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				int spread = 1;
				if (rush)
				{
					if (m_Difficulty == Difficulty::Lunatic) spread = 3;
					else if (m_Difficulty == Difficulty::Hard) spread = 2;
					else spread = 2;
					if (m_Difficulty == Difficulty::Easy) spread = 1;
				}
				for (int n = -spread; n <= spread; n++)
				{
					float off = (float)n * 0.15f;
					bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						(ndx * cosf(off) - ndz * sinf(off)) * bulletSpd * 1.2f, 0.0f,
						(ndx * sinf(off) + ndz * cosf(off)) * bulletSpd * 1.2f,
						bulletRad, GetColor(255, 120, 200));
				}
			}
		}
	}
	else if (m_BossPhase == 1)
	{
		const int spiralInt = rush ? bossInterval(2, 2, 0) : (2 + fireIntervalScale);
		if (m_BossPatternTimer % spiralInt == 0)
		{
			float angle1 = (float)m_BossPatternTimer * 0.13f;
			const int arms = rush ? bossCount(3, 2) : 2;
			for (int k = 0; k < arms; k++)
			{
				float a = angle1 + (float)k * (3.14159265f * 2.0f / arms);
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * bulletSpd, 0.0f, sinf(a) * bulletSpd,
					bulletRad * 0.95f, GetColor(255, 200, 50));
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a + 0.4f) * bulletSpd * 0.85f, 0.0f, sinf(a + 0.4f) * bulletSpd * 0.85f,
					bulletRad * 0.85f, GetColor(255, 255, 120));
			}
		}

		const int radialInt = bossInterval(20, 28, 8);
		if (m_BossPatternTimer % radialInt == 0)
		{
			const int count = bossCount(32, 24);
			float spin = (float)m_BossPatternTimer * 0.03f;
			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + spin;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * bulletSpd * 0.8f, 0.0f, sinf(angle) * bulletSpd * 0.8f,
					bulletRad * 1.1f, GetColor(255, 80, 255));
			}
		}
	}
	else if (m_BossPhase == 2)
	{
		const int rainInterval = rush ? bossInterval(3, 4, 0) : 4;
		if (m_BossPatternTimer % rainInterval == 0)
		{
			float rx = (float)(GetRand(800) - 400);
			bullets.AddEnemyBullet(rx, m_BossY, FIELD_HALF_D + 50.0f,
				0.0f, 0.0f, -bulletSpd * 1.1f, bulletRad * 0.85f, GetColor(0, 255, 255));
		}

		const int radialInterval = bossInterval(14, 18, 6);
		if (m_BossPatternTimer % radialInterval == 0)
		{
			const int count = bossCount(36, 32);
			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + ((float)m_BossPatternTimer * 0.04f);
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * bulletSpd * 0.78f, 0.0f, sinf(angle) * bulletSpd * 0.78f,
					bulletRad * 1.15f, GetColor(255, 0, 255));
			}
		}

		const int aimedInterval = rush ? bossInterval(8, 10, 0) : 10;
		if (m_BossPatternTimer % aimedInterval == 0)
		{
			float dx = playerX - m_BossX;
			float dz = playerZ - m_BossZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				int spread = rush ? bossCount(4, 3) : 3;
				for (int n = -spread; n <= spread; n++)
				{
					float off = (float)n * 0.1f;
					bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						(ndx * cosf(off) - ndz * sinf(off)) * bulletSpd * 1.25f, 0.0f,
						(ndx * sinf(off) + ndz * cosf(off)) * bulletSpd * 1.25f,
						bulletRad * 1.2f, GetColor(255, 40, 80));
				}
			}
		}
	}
	else // m_BossPhase >= 3 : 第4形態（ボスラッシュ専用）
	{
		const int spiralInt = rush ? bossInterval(3, 3, 0) : 3;
		if (m_BossPatternTimer % spiralInt == 0)
		{
			float a1 = (float)m_BossPatternTimer * 0.18f;
			const int arms = rush ? bossCount(4, 4) : 4;
			for (int k = 0; k < arms; k++)
			{
				float a = a1 + (float)k * 1.5708f;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * bulletSpd * 1.05f, 0.0f, sinf(a) * bulletSpd * 1.05f,
					bulletRad * 1.05f, GetColor(255, 80, 80));
			}
		}

		const int radialInt = rush ? bossInterval(12, 12, 0) : 12;
		if (m_BossPatternTimer % radialInt == 0)
		{
			const int count = bossCount(28, 28);
			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + ((float)m_BossPatternTimer * 0.07f);
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * bulletSpd * 0.82f, 0.0f, sinf(angle) * bulletSpd * 0.82f,
					bulletRad * 1.1f, GetColor(255, 60, 200));
			}
		}

		const int aimedInt = rush ? bossInterval(6, 6, 0) : 6;
		if (m_BossPatternTimer % aimedInt == 0)
		{
			float dx = playerX - m_BossX;
			float dz = playerZ - m_BossZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				int spread = rush ? bossCount(2, 2) : 2;
				for (int n = -spread; n <= spread; n++)
				{
					float off = (float)n * 0.12f;
					bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						(ndx * cosf(off) - ndz * sinf(off)) * bulletSpd * 1.3f, 0.0f,
						(ndx * sinf(off) + ndz * cosf(off)) * bulletSpd * 1.3f,
						bulletRad * 1.15f, GetColor(255, 255, 100));
				}
			}
		}

		const int rainInt = rush ? bossInterval(4, 4, 0) : 4;
		if (m_BossPatternTimer % rainInt == 0)
		{
			float rx = (float)(GetRand(800) - 400);
			bullets.AddEnemyBullet(rx, m_BossY, FIELD_HALF_D + 80.0f,
				0.0f, 0.0f, -bulletSpd * 1.15f, bulletRad * 0.9f, GetColor(0, 255, 200));
		}
	}
}

// --- Update: 全体ウェーブとザコ敵・ボスの統括更新 ---
void EnemyManager::Update(float playerX, float playerZ, BulletManager& bullets)
{
	if (m_AllWavesComplete) return;

	if (m_BossHitCooldown > 0) m_BossHitCooldown--;
	if (m_MidBossHitCooldown > 0) m_MidBossHitCooldown--;

	int activeEnemyCount = 0;
	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (m_Enemies[i].IsActive())
		{
			m_Enemies[i].Update(playerX, playerZ, bullets);
			activeEnemyCount++;
		}
	}

	if (m_SpellBreakTimer > 0)
		m_SpellBreakTimer--;

	if (m_BossActive)
	{
		if (m_BossIntroTimer > 0)
		{
			m_BossIntroTimer--;
			if (!m_BossIntroClearedBullets)
			{
				bullets.ClearEnemyBullets();
				m_BossIntroClearedBullets = true;
			}
		}
		else
		{
			UpdateBossMovement();
			if (m_SpellBreakTimer <= 0)
				UpdateBoss(playerX, playerZ, bullets);
		}
		return;
	}

	if (m_MidBossActive)
	{
		UpdateMidBoss(playerX, playerZ, bullets);
		return;
	}

	UpdateWaveDanmaku(playerX, playerZ, bullets);

	if (m_WaveTimer > 0)
	{
		m_WaveTimer--;
		return;
	}

	// ウェーブ別出現スケジュール
	switch (m_WaveIndex)
	{
	case 0:
		// WAVE 1: ザコ（基本型）大量出現で爽快導入
		if (m_SpawnCount < 14)
		{
			m_SpawnTimer++;
			if (m_SpawnTimer >= 20)
			{
				m_SpawnTimer = 0;
				float sx = (float)(GetRand(500) - 250);
				SpawnEnemy(PickStageEnemy(m_StageChapter, 0, m_SpawnCount), sx, 450.0f);
				m_SpawnCount++;
			}
		}
		else if (activeEnemyCount == 0)
			OnWaveSectionClear();
		break;

	case 1:
		// WAVE 2: スピナー＋基本型の混成ウェーブ
		if (m_SpawnCount < 12)
		{
			m_SpawnTimer++;
			if (m_SpawnTimer >= 30)
			{
				m_SpawnTimer = 0;
				float sx = (float)(GetRand(400) - 200);
				EnemyType type = PickStageEnemy(m_StageChapter, 1, m_SpawnCount);
				SpawnEnemy(type, sx, 450.0f);
				m_SpawnCount++;
			}
		}
		else if (activeEnemyCount == 0)
			OnWaveSectionClear();
		break;

	case 2:
		// WAVE 3: 耐久タンク＋基本型の混合フォーメーション
		if (m_SpawnCount == 0)
		{
			SpawnEnemy(EnemyType::Tank,    -120.0f, 480.0f);
			SpawnEnemy(EnemyType::Tank,     120.0f, 480.0f);
			SpawnEnemy(EnemyType::Basic,   -260.0f, 450.0f);
			SpawnEnemy(EnemyType::Basic,      0.0f, 450.0f);
			SpawnEnemy(EnemyType::Basic,    260.0f, 450.0f);
			SpawnEnemy(EnemyType::Spinner, -360.0f, 430.0f);
			SpawnEnemy(EnemyType::Spinner,  360.0f, 430.0f);
			m_SpawnCount = 7;
		}
		else if (activeEnemyCount == 0)
			OnWaveSectionClear();
		break;

	case 3:
		// WAVE 4: 二段階の大規模波状攻撃ウェーブ（前哨戦）
		if (m_SpawnCount == 0)
		{
			SpawnEnemy(EnemyType::Tank,       0.0f, 500.0f);
			SpawnEnemy(EnemyType::Spinner, -200.0f, 470.0f);
			SpawnEnemy(EnemyType::Spinner,  200.0f, 470.0f);
			SpawnEnemy(EnemyType::Basic,   -320.0f, 450.0f);
			SpawnEnemy(EnemyType::Basic,   -160.0f, 450.0f);
			SpawnEnemy(EnemyType::Basic,    160.0f, 450.0f);
			SpawnEnemy(EnemyType::Basic,    320.0f, 450.0f);
			m_SpawnCount = 7;
		}
		else
		{
			m_SpawnTimer++;
			if (m_SpawnCount < 13 && m_SpawnTimer >= 55)
			{
				m_SpawnTimer = 0;
				float sx = (float)(GetRand(500) - 250);
				EnemyType type = (m_SpawnCount % 3 == 0) ? EnemyType::Spinner : EnemyType::Basic;
				SpawnEnemy(type, sx, 460.0f);
				m_SpawnCount++;
			}
			if (activeEnemyCount == 0 && m_SpawnCount >= 13)
				OnWaveSectionClear();
		}
		break;

	case 4:
		// WAVE 5: 巨大ボス出現！
		if (!m_BossActive && !m_AllWavesComplete)
		{
			SpawnBoss();
		}
		break;
	}
}

// --- Draw: ザコ敵およびボス本体の3D描画 ---
void EnemyManager::Draw() const
{
	if (m_AllWavesComplete) return;

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		if (m_Enemies[i].IsActive())
		{
			m_Enemies[i].Draw();
		}
	}

	if (m_MidBossActive)
		DrawMidBoss();
	if (m_BossActive)
		DrawBoss();
}

void EnemyManager::DrawMidBoss() const
{
	if (!m_MidBossActive) return;

	if (PreferGameSprites() && ResourceManager::IsReady())
	{
		ResourceManager::DrawMidBoss(m_MidBossX, PLAYER_Y, m_MidBossZ, m_MidBossRadius, m_MidBossId, m_MidBossTimer);
		return;
	}

	unsigned int col = (m_MidBossId == 1) ? GetColor(255, 100, 180) : GetColor(255, 180, 60);
	VECTOR c = VGet(m_MidBossX, PLAYER_Y, m_MidBossZ);

	// [VISUAL_RICH] 脈動ハロー＋同心ライト感
	if (VISUAL_RICH)
	{
		float pulse = sinf((float)GetNowCount() / 130.0f) * 4.0f;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 110);
		DrawSphere3D(c, m_MidBossRadius * 1.55f + pulse, 8, col, col, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 60);
		DrawSphere3D(c, m_MidBossRadius * 2.1f + pulse * 1.4f, 8, col, col, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// [VISUAL_STYLE] / [VISUAL_THEME] 中ボス
	if (UseTouhouTheme() && UseSilhouetteStyle())
	{
		SilhouetteDraw::DrawMidBossYoukai(m_MidBossX, PLAYER_Y, m_MidBossZ, m_MidBossRadius,
			m_MidBossId, GetColor(40, 10, 50), col, m_MidBossTimer);
	}
	else if (UseSilhouetteStyle())
	{
		SilhouetteDraw::DrawMidBoss(m_MidBossX, PLAYER_Y, m_MidBossZ, m_MidBossRadius,
			m_MidBossId, GetColor(40, 10, 50), col, m_MidBossTimer);
	}
	else
	{
		DrawSphere3D(c, m_MidBossRadius, 8, GetColor(40, 10, 50), col, FALSE);
	}
}

// --- DrawBoss: 最終巨大ボスの極彩色3D描画 ---
void EnemyManager::DrawBoss() const
{
	if (!m_BossActive) return;

	if (PreferGameSprites() && ResourceManager::IsReady())
	{
		ResourceManager::DrawBoss(m_BossX, m_BossY, m_BossZ, m_BossDrawRadius, m_BossPhase, m_BossTimer);
		return;
	}

	unsigned int bodyColor = GetColor(15, 5, 30);
	unsigned int edgeColor;

	if (m_BossPhase == 0)       edgeColor = GetColor(255, 50, 150);
	else if (m_BossPhase == 1)  edgeColor = GetColor(255, 180, 0);
	else                        edgeColor = GetColor(255, 0, 50);

	VECTOR bossCenter = VGet(m_BossX, m_BossY, m_BossZ);

	// [VISUAL_RICH] ボス周囲に脈動する巨大な2層オーラ
	if (VISUAL_RICH)
	{
		float pulse = sinf((float)m_BossTimer / 14.0f) * 6.0f;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
		DrawSphere3D(bossCenter, m_BossDrawRadius * 1.35f + pulse, 8, edgeColor, edgeColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 60);
		DrawSphere3D(bossCenter, m_BossDrawRadius * 1.85f + pulse * 1.3f, 8, edgeColor, edgeColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// [VISUAL_STYLE] / [VISUAL_THEME] ボス
	if (UseTouhouTheme() && UseSilhouetteStyle())
	{
		SilhouetteDraw::DrawBossYoukai(m_BossX, m_BossY, m_BossZ, m_BossDrawRadius, m_BossPhase,
			bodyColor, edgeColor, m_BossTimer);
	}
	else if (UseSilhouetteStyle())
	{
		SilhouetteDraw::DrawBoss(m_BossX, m_BossY, m_BossZ, m_BossDrawRadius, m_BossPhase,
			bodyColor, edgeColor, m_BossTimer);
	}
	else
	{
		VECTOR minPos = VGet(m_BossX - m_BossDrawRadius, m_BossY - m_BossDrawRadius, m_BossZ - m_BossDrawRadius);
		VECTOR maxPos = VGet(m_BossX + m_BossDrawRadius, m_BossY + m_BossDrawRadius, m_BossZ + m_BossDrawRadius);
		DrawCube3D(minPos, maxPos, bodyColor, edgeColor, FALSE);
		float coreRadius = m_BossDrawRadius * 0.65f + sinf((float)m_BossTimer / 10.0f) * 3.0f;
		DrawSphere3D(bossCenter, coreRadius, 8, edgeColor, edgeColor, FALSE);
	}
}