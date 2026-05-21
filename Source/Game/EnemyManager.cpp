#include "Game/EnemyManager.h"
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
	m_BossRadius = 40.0f;
	m_BossHp = 0;
	m_BossMaxHp = 0;
	m_BossTimer = 0;
	m_BossPhase = 0;
	m_WaveDanmakuTimer = 0;
	m_SpellBreakTimer = 0;
	m_BossHitCooldown = 0;
	m_MidBossHitCooldown = 0;
	m_StageChapter = 0;
	m_BossIntroTimer = 0;
	m_BossIntroClearedBullets = false;
	m_MidBossActive = false;
	m_MidBossId = 0;
	m_MidBossHp = 0;
	m_EnableMidBoss1 = true;
	m_EnableMidBoss2 = true;
	m_PlayMode = PlayMode::Story;

	for (int i = 0; i < ENEMY_MAX; i++)
	{
		m_Enemies[i].Deactivate();
	}
}

void EnemyManager::ConfigureStage(StageStart stage, PlayMode mode, int stageChapter)
{
	m_PlayMode = mode;
	m_StageChapter = stageChapter;
	if (m_StageChapter < 0) m_StageChapter = 0;
	if (m_StageChapter >= STAGE_CHAPTER_COUNT) m_StageChapter = STAGE_CHAPTER_COUNT - 1;
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
	m_WaveTimer = 70;
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
		if (m_BossPhase >= 3)
		{
			m_BossActive = false;
			m_AllWavesComplete = true;
		}
		else
		{
			m_SpellBreakTimer = SPELL_BREAK_FRAMES;
			m_BossTimer = 0;
			bullets.ClearEnemyBullets();
			if (m_BossPhase == 1)
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
	m_MidBossRadius = 28.0f;
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

// --- SpawnBoss: 最終ウェーブでのボス出現 ---
void EnemyManager::SpawnBoss()
{
	m_BossActive = true;
	m_BossX = 0.0f;
	m_BossY = PLAYER_Y;
	m_BossZ = 400.0f;
	m_BossRadius = 38.0f;

	m_BossHp = (m_StageChapter >= 1) ? 110 : 90;
	m_BossMaxHp = m_BossHp;
	m_BossPhase = 0;
	m_BossTimer = 0;
	m_BossIntroTimer = BOSS_INTRO_FRAMES;
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
		if (m_WaveDanmakuTimer % 36 == 0)
			ringBurst(10, 0.95f, GetColor(255, 80, 180), (float)m_WaveDanmakuTimer * 0.035f);
		if (m_WaveDanmakuTimer % 10 == 0)
		{
			for (int side = -1; side <= 1; side += 2)
			{
				bullets.AddEnemyBullet(side * 360.0f, PLAYER_Y, FIELD_HALF_D - 40.0f,
					0.0f, 0.0f, -spd * 1.05f, smallR, GetColor(0, 220, 255));
				bullets.AddEnemyBullet(side * 280.0f, PLAYER_Y, FIELD_HALF_D - 80.0f,
					side * 0.4f, 0.0f, -spd, smallR, GetColor(120, 255, 255));
			}
		}
		break;

	case 1:
		if (m_WaveDanmakuTimer % 28 == 0)
			ringBurst(14, 0.88f, GetColor(255, 200, 60), (float)m_WaveDanmakuTimer * 0.06f);
		if (m_WaveDanmakuTimer % 32 == 0)
		{
			float dx = playerX - emitX;
			float dz = playerZ - emitZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				for (int n = -2; n <= 2; n++)
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
		if (m_WaveDanmakuTimer % 12 == 0)
		{
			float a1 = (float)m_WaveDanmakuTimer * 0.14f;
			float a2 = a1 + pi;
			bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ, cosf(a1) * spd, 0.0f, sinf(a1) * spd, smallR, GetColor(255, 120, 255));
			bullets.AddEnemyBullet(emitX, PLAYER_Y, emitZ, cosf(a2) * spd, 0.0f, sinf(a2) * spd, smallR, GetColor(255, 120, 255));
			bullets.AddEnemyBullet(-emitX, PLAYER_Y, emitZ, cosf(a1 + 0.5f) * spd, 0.0f, sinf(a1 + 0.5f) * spd, smallR, GetColor(200, 80, 255));
			bullets.AddEnemyBullet(-emitX, PLAYER_Y, emitZ, cosf(a2 + 0.5f) * spd, 0.0f, sinf(a2 + 0.5f) * spd, smallR, GetColor(200, 80, 255));
		}
		if (m_WaveDanmakuTimer % 52 == 0)
			ringBurst(16, 0.75f, GetColor(255, 0, 120), (float)m_WaveDanmakuTimer * 0.02f);
		break;

	case 3:
		if (m_WaveDanmakuTimer % 8 == 0)
		{
			float rx = (float)(GetRand(700) - 350);
			bullets.AddEnemyBullet(rx, PLAYER_Y, FIELD_HALF_D + 30.0f,
				0.0f, 0.0f, -spd * 1.15f, smallR * 0.9f, GetColor(0, 255, 220));
		}
		if (m_WaveDanmakuTimer % 34 == 0)
			ringBurst(18, 0.82f, GetColor(255, 50, 255), (float)m_WaveDanmakuTimer * 0.045f);
		if (m_WaveDanmakuTimer % 16 == 0)
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

	if (m_StageChapter >= 1 && m_WaveDanmakuTimer % 20 == 0)
	{
		float twist = (float)m_WaveDanmakuTimer * 0.08f;
		float lane = (float)((GetRand(1000) % 7) - 3) * 70.0f;
		bullets.AddEnemyBullet(lane, PLAYER_Y, FIELD_HALF_D - 20.0f,
			sinf(twist) * spd * 0.6f, 0.0f, -spd * 0.95f,
			smallR * 0.9f, GetColor(180, 80, 255));
	}
}

// --- UpdateBoss: ボスの行動とスペルカード攻撃パターン ---
void EnemyManager::UpdateBoss(float playerX, float playerZ, BulletManager& bullets)
{
	if (!m_BossActive || m_BossIntroTimer > 0) return;

	m_BossTimer++;

	// ボスの左右緩やかな飛行移動（の字移動）
	m_BossX = sinf((float)m_BossTimer / 45.0f) * 160.0f;
	m_BossZ = 350.0f + cosf((float)m_BossTimer / 60.0f) * 35.0f;

	// 難易度による発射ペースの補正
	int fireIntervalScale = 1;
	if (m_Difficulty == Difficulty::Easy)   fireIntervalScale = 2;
	if (m_Difficulty == Difficulty::Hard)   fireIntervalScale = 0;

	float m_BulletSpeed = EBULLET_SPEED_NORMAL;
	if (m_Difficulty == Difficulty::Easy) m_BulletSpeed = EBULLET_SPEED_EASY;
	if (m_Difficulty == Difficulty::Hard) m_BulletSpeed = EBULLET_SPEED_HARD;

	// --- 段階別攻撃パターン（スペルカード） ---
	if (m_BossPhase == 0)
	{
		int interval = 22 + fireIntervalScale * 6;
		if (m_BossTimer % interval == 0)
		{
			int count = 20;
			if (m_Difficulty == Difficulty::Easy) count = 14;
			if (m_Difficulty == Difficulty::Hard) count = 26;

			float offsetAngle = (m_BossTimer * 0.055f);
			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + offsetAngle;
				float vx = cosf(angle) * m_BulletSpeed * 0.9f;
				float vz = sinf(angle) * m_BulletSpeed * 0.9f;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ, vx, 0.0f, vz,
					EBULLET_RADIUS * 1.05f, GetColor(255, 50, 150));
			}
		}
		if (m_BossTimer % 14 == 0)
		{
			float dx = playerX - m_BossX;
			float dz = playerZ - m_BossZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				for (int n = -1; n <= 1; n++)
				{
					float off = (float)n * 0.15f;
					bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						(ndx * cosf(off) - ndz * sinf(off)) * m_BulletSpeed * 1.2f, 0.0f,
						(ndx * sinf(off) + ndz * cosf(off)) * m_BulletSpeed * 1.2f,
						EBULLET_RADIUS, GetColor(255, 120, 200));
				}
			}
		}
	}
	else if (m_BossPhase == 1)
	{
		if (m_BossTimer % (3 + fireIntervalScale) == 0)
		{
			float angle1 = (float)m_BossTimer * 0.13f;
			float angle2 = angle1 + 3.14159265f;
			for (int k = 0; k < 2; k++)
			{
				float a = (k == 0) ? angle1 : angle2;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * m_BulletSpeed, 0.0f, sinf(a) * m_BulletSpeed,
					EBULLET_RADIUS * 0.95f, GetColor(255, 200, 50));
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a + 0.4f) * m_BulletSpeed * 0.85f, 0.0f, sinf(a + 0.4f) * m_BulletSpeed * 0.85f,
					EBULLET_RADIUS * 0.85f, GetColor(255, 255, 120));
			}
		}

		if (m_BossTimer % (28 + fireIntervalScale * 8) == 0)
		{
			int count = 24;
			float spin = (float)m_BossTimer * 0.03f;
			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + spin;
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * m_BulletSpeed * 0.8f, 0.0f, sinf(angle) * m_BulletSpeed * 0.8f,
					EBULLET_RADIUS * 1.1f, GetColor(255, 80, 255));
			}
		}
	}
	else if (m_BossPhase == 2)
	{
		if (m_BossTimer % 6 == 0)
		{
			float rx = (float)(GetRand(800) - 400);
			bullets.AddEnemyBullet(rx, m_BossY, FIELD_HALF_D + 50.0f,
				0.0f, 0.0f, -m_BulletSpeed * 1.1f, EBULLET_RADIUS * 0.85f, GetColor(0, 255, 255));
		}

		int radialInterval = 18 + fireIntervalScale * 6;
		if (m_BossTimer % radialInterval == 0)
		{
			int count = 22;
			if (m_Difficulty == Difficulty::Easy) count = 16;
			if (m_Difficulty == Difficulty::Hard) count = 28;

			for (int i = 0; i < count; i++)
			{
				float angle = (i * 2.0f * 3.14159265f) / count + ((float)m_BossTimer * 0.04f);
				bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(angle) * m_BulletSpeed * 0.78f, 0.0f, sinf(angle) * m_BulletSpeed * 0.78f,
					EBULLET_RADIUS * 1.15f, GetColor(255, 0, 255));
			}
		}

		if (m_BossTimer % 10 == 0)
		{
			float dx = playerX - m_BossX;
			float dz = playerZ - m_BossZ;
			float dist = sqrtf(dx * dx + dz * dz);
			if (dist > 1.0f)
			{
				float ndx = dx / dist;
				float ndz = dz / dist;
				for (int n = -3; n <= 3; n++)
				{
					float off = (float)n * 0.1f;
					bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						(ndx * cosf(off) - ndz * sinf(off)) * m_BulletSpeed * 1.25f, 0.0f,
						(ndx * sinf(off) + ndz * cosf(off)) * m_BulletSpeed * 1.25f,
						EBULLET_RADIUS * 1.2f, GetColor(255, 40, 80));
				}
			}
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
		else if (m_SpellBreakTimer <= 0)
			UpdateBoss(playerX, playerZ, bullets);
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
	unsigned int col = (m_MidBossId == 1) ? GetColor(255, 100, 180) : GetColor(255, 180, 60);
	VECTOR c = VGet(m_MidBossX, PLAYER_Y, m_MidBossZ);
	DrawSphere3D(c, m_MidBossRadius, 8, GetColor(40, 10, 50), col, TRUE);
}

// --- DrawBoss: 最終巨大ボスの極彩色3D描画 ---
void EnemyManager::DrawBoss() const
{
	if (!m_BossActive) return;

	VECTOR minPos = VGet(m_BossX - m_BossRadius, m_BossY - m_BossRadius, m_BossZ - m_BossRadius);
	VECTOR maxPos = VGet(m_BossX + m_BossRadius, m_BossY + m_BossRadius, m_BossZ + m_BossRadius);

	unsigned int bodyColor = GetColor(15, 5, 30);
	unsigned int edgeColor;

	if (m_BossPhase == 0)       edgeColor = GetColor(255, 50, 150);
	else if (m_BossPhase == 1)  edgeColor = GetColor(255, 180, 0);
	else                        edgeColor = GetColor(255, 0, 50);

	DrawCube3D(minPos, maxPos, bodyColor, edgeColor, TRUE);

	float coreRadius = m_BossRadius * 0.65f + sinf((float)m_BossTimer / 10.0f) * 3.0f;
	DrawSphere3D(VGet(m_BossX, m_BossY, m_BossZ), coreRadius, 8, edgeColor, edgeColor, FALSE);
}