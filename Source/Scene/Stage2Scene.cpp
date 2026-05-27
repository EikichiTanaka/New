// ============================================================
// Stage2Scene.cpp
// Stage 2: TPS-style 3D boss fight (boss-only).
// (Japanese UI strings use \uXXXX escapes to keep the source ASCII.)
// ============================================================

#include "Scene/Stage2Scene.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/GameOptions.h"
#include "Common/GameScreen.h"
#include "Common/KeyHelper.h"
#include "Common/UiDraw.h"
#include "Common/BgmPlayer.h"
#include "Common/SoundSynth.h"
#include "Common/SaveData.h"
#include "Game/Collision.h"
#include "GameConfig.h"
#include "DxLib.h"
#include <cmath>

namespace
{
constexpr float PI_F            = 3.14159265f;
constexpr float TWO_PI_F        = 6.28318530f;
constexpr float STAGE2_PLAYER_Y = 60.0f;
constexpr float MOUSE_SENSITIVITY = 0.0038f;
constexpr float PITCH_LIMIT       = 1.35f;

constexpr float PLAYER_ACCEL         = 0.55f;
constexpr float PLAYER_DRAG          = 0.86f;
constexpr float PLAYER_MAX_SPEED     = 6.2f;
constexpr float PLAYER_MAX_SPEED_SLOW = 2.6f;
constexpr float PLAYER_BOUND_X       = 420.0f;
constexpr float PLAYER_BOUND_Y       = 320.0f;
constexpr float PLAYER_BOUND_Z       = 420.0f;
constexpr float PLAYER_CENTER_Y      = 60.0f;

constexpr int   STAGE2_FIRE_INTERVAL       = 4;
constexpr int   STAGE2_FIRE_INTERVAL_SLOW  = 3;
constexpr float STAGE2_PLAYER_BULLET_SPEED  = 34.0f;
constexpr float STAGE2_PLAYER_BULLET_RADIUS = 6.2f;

constexpr int   STAGE2_INV_FRAMES   = 90;
constexpr float STAGE2_PLAYER_RADIUS = 9.0f;
constexpr float STAGE2_PLAYER_SHIP_DRAW_R = 14.0f;

constexpr int   STAGE2_START_LIVES = 4;
constexpr int   STAGE2_START_BOMBS = 3;

constexpr int   STAGE2_KILL_CHAIN_FRAMES = 90;

// Boss params
constexpr float BOSS_RADIUS_BASE       = 46.0f;   // phase 0 base radius
constexpr float BOSS_RADIUS_PER_PHASE  = 4.0f;
constexpr float BOSS_HIT_RADIUS_MUL    = 0.72f;
constexpr float BOSS_ORBIT_MIN_R       = 320.0f;
constexpr float BOSS_ORBIT_MAX_R       = 720.0f;
constexpr int   STAGE2_BOSS_INTRO_FRAMES = 70;

// Lightweight tuning (lower seg counts to reduce GPU load)
constexpr int   STAGE2_SPHERE_SEG_HI   = 10;
constexpr int   STAGE2_SPHERE_SEG_LO   = 6;
constexpr int   STAGE2_SKYBOX_COUNT    = 36;

inline float Clampf(float v, float lo, float hi)
{
	if (v < lo) return lo;
	if (v > hi) return hi;
	return v;
}

inline float FrandRange(float lo, float hi)
{
	const int span = (int)((hi - lo) * 1000.0f);
	if (span <= 0) return lo;
	return lo + (float)GetRand(span) / 1000.0f;
}

inline void NormalizeVec3(float& x, float& y, float& z)
{
	const float L = sqrtf(x * x + y * y + z * z);
	if (L > 0.0001f) { x /= L; y /= L; z /= L; }
}

inline float Stage2BulletSpeedMul(Difficulty d)
{
	return GetBossRushTune(d).bulletSpeedMul;
}

inline int Stage2FireIntervalScale(int base, Difficulty d)
{
	return BossRushScaleFireInterval(base, d);
}

inline float BossPhaseRadius(int phase)
{
	if (phase < 0) phase = 0;
	if (phase >= BOSS_RUSH_PHASE_COUNT) phase = BOSS_RUSH_PHASE_COUNT - 1;
	return BOSS_RADIUS_BASE + (float)phase * BOSS_RADIUS_PER_PHASE;
}

inline unsigned int BossPhaseColor(int phase)
{
	switch (phase)
	{
	case 0: return GetColor(180, 220, 255);
	case 1: return GetColor(255, 200, 130);
	case 2: return GetColor(255, 130, 200);
	default: return GetColor(255,  90, 110);
	}
}

inline unsigned int BossBulletPhaseColor(int phase, int variant)
{
	switch (phase)
	{
	case 0: return (variant & 1) ? GetColor(160, 220, 255) : GetColor(200, 240, 255);
	case 1: return (variant & 1) ? GetColor(255, 200, 130) : GetColor(255, 230, 170);
	case 2: return (variant & 1) ? GetColor(255, 140, 220) : GetColor(255, 200, 240);
	default: return (variant & 1) ? GetColor(255,  90, 110) : GetColor(255, 160, 180);
	}
}

} // namespace

Stage2Scene::~Stage2Scene()
{
	DisableSceneLighting();
	LeaveStage();
}

void Stage2Scene::Init()
{
	GameScreenSyncSize();

	m_Bullets.Init();
	m_Effect.Init();

	m_PlayerX = 0.0f;
	m_PlayerY = STAGE2_PLAYER_Y;
	m_PlayerZ = 0.0f;
	m_PlayerVX = m_PlayerVY = m_PlayerVZ = 0.0f;
	m_PlayerSpeed = 0.0f;
	m_Yaw   = 0.0f;
	m_Pitch = 0.0f;
	m_CamAnchorX = m_PlayerX;
	m_CamAnchorY = m_PlayerY;
	m_CamAnchorZ = m_PlayerZ;

	const bool practice = (g_Session.playMode == PlayMode::Practice);
	const bool scoreAtk = (g_Session.playMode == PlayMode::ScoreAttack);
	m_Lives = (practice || scoreAtk) ? 99 : STAGE2_START_LIVES;
	m_BombCount = STAGE2_START_BOMBS;
	m_Score = 0;
	m_GrazeCount = 0;
	m_KillChain = 0;
	m_MaxKillChain = 0;
	m_KillChainTimer = 0;
	m_FireCooldown = 0;
	m_InvTimer = 0;
	m_BombsUsed = 0;
	m_Alive = true;
	m_BombKeyHeld = false;
	m_ComboCount = 0;
	m_ComboTimer = 0;
	m_BossHitPopTimer = 0;
	m_BossHitPopScore = 0;

	// Boss state
	m_BossPhase = ClampBossRushStartPhase(g_Session.bossRushStartPhase);
	m_BossPracticeMode = practice;
	m_BossMaxHp = GetBossRushPhaseHp(g_GameData.difficulty, m_BossPhase, practice);
	m_BossHp = m_BossMaxHp;
	m_BossX = 0.0f;
	m_BossY = m_PlayerY + 80.0f;
	m_BossZ = 500.0f;
	m_BossVX = m_BossVY = m_BossVZ = 0.0f;
	m_BossTX = m_BossX;
	m_BossTY = m_BossY;
	m_BossTZ = m_BossZ;
	m_BossDrawRadius = BossPhaseRadius(m_BossPhase);
	m_BossHitRadius  = m_BossDrawRadius * BOSS_HIT_RADIUS_MUL;
	m_BossTimer = 0;
	m_BossPatternTimer = 0;
	m_SpellBreakTimer = 0;
	m_BossIntroTimer = STAGE2_BOSS_INTRO_FRAMES;
	m_BossHitCooldown = 0;
	m_BossHitFlash = 0;
	m_BossActive = true;
	m_BossDefeated = false;

	m_FrameCount = 0;
	m_ClearDelayTimer = 0;
	m_HitStopTimer = 0;
	m_ShakeTimer = 0;
	m_ShakeMag = 0.0f;
	m_DamageFlashTimer = 0;
	m_BombFlashTimer = 0;
	m_SpellBannerTimer = 120;

	m_Paused = false;
	m_PauseMenuCursor = 0;

	m_DirLightHandle = -1;
	m_LightingActive = false;

	g_GameData.score = 0;
	g_GameData.livesLeft = m_Lives;
	g_GameData.playTimeSec = 0;
	g_GameData.grazeCount = 0;
	g_GameData.isClear = false;
	g_GameData.isGameOver = false;
	g_GameData.runStats = RunStats{};
	g_GameData.runStats.bossMaxPhaseReached = m_BossPhase;
	g_GameData.scoreMultiplier = 1.0f;
	g_GameData.spellBonusCollected = 0;

	SetupSceneLighting();
	EnterStage();
	BgmPlayer::Play(BgmTrack::Boss);
}

void Stage2Scene::EnterStage()
{
	m_MouseCenterX = SCREEN_WIDTH / 2;
	m_MouseCenterY = SCREEN_HEIGHT / 2;
	SetMouseDispFlag(FALSE);
	SetMousePoint(m_MouseCenterX, m_MouseCenterY);
	m_MouseCaptured = true;
	m_PendingMouseRecenter = true;
}

void Stage2Scene::LeaveStage()
{
	if (m_MouseCaptured)
	{
		SetMouseDispFlag(TRUE);
		m_MouseCaptured = false;
	}
}

void Stage2Scene::RecenterMouse()
{
	m_MouseCenterX = SCREEN_WIDTH / 2;
	m_MouseCenterY = SCREEN_HEIGHT / 2;
	SetMousePoint(m_MouseCenterX, m_MouseCenterY);
}

void Stage2Scene::SetupSceneLighting()
{
	SetUseLighting(TRUE);
	SetGlobalAmbientLight(GetColorF(0.40f, 0.44f, 0.52f, 1.0f));
	m_DirLightHandle = CreateDirLightHandle(VGet(0.25f, -0.55f, 0.75f));
	SetLightDifColorHandle(m_DirLightHandle, GetColorF(0.95f, 0.92f, 0.88f, 1.0f));
	SetLightAmbColorHandle(m_DirLightHandle, GetColorF(0.40f, 0.46f, 0.55f, 1.0f));
	SetLightSpcColorHandle(m_DirLightHandle, GetColorF(0.3f, 0.3f, 0.35f, 1.0f));
	m_LightingActive = true;
}

void Stage2Scene::DisableSceneLighting()
{
	if (m_LightingActive)
	{
		if (m_DirLightHandle != -1)
		{
			DeleteLightHandle(m_DirLightHandle);
			m_DirLightHandle = -1;
		}
		SetUseLighting(FALSE);
		m_LightingActive = false;
	}
}

void Stage2Scene::ComputeForward(float& fx, float& fy, float& fz) const
{
	const float cp = cosf(m_Pitch);
	fx = sinf(m_Yaw) * cp;
	fy = sinf(m_Pitch);
	fz = cosf(m_Yaw) * cp;
}

void Stage2Scene::ComputeRight(float& rx, float& ry, float& rz) const
{
	rx = cosf(m_Yaw);
	ry = 0.0f;
	rz = -sinf(m_Yaw);
}

SceneType Stage2Scene::Update()
{
	m_FrameCount++;

	if (!m_Paused)
	{
		if (KeyHelper::IsTrigger(KEY_INPUT_P) || KeyHelper::IsTrigger(KEY_INPUT_ESCAPE))
		{
			m_Paused = true;
			m_PauseMenuCursor = 0;
			SetMouseDispFlag(TRUE);
			return SceneType::None;
		}
	}
	else
	{
		return UpdatePauseMenu();
	}

	if (m_HitStopTimer > 0)
	{
		m_HitStopTimer--;
		m_Effect.Update();
		UpdateScreenShake();
		return SceneType::None;
	}

	if (m_DamageFlashTimer > 0) m_DamageFlashTimer--;
	if (m_BombFlashTimer > 0) m_BombFlashTimer--;
	if (m_SpellBannerTimer > 0) m_SpellBannerTimer--;
	if (m_BossIntroTimer > 0) m_BossIntroTimer--;
	if (m_BossHitFlash > 0) m_BossHitFlash--;
	if (m_BossHitCooldown > 0) m_BossHitCooldown--;
	if (m_ComboTimer > 0)
	{
		m_ComboTimer--;
		if (m_ComboTimer == 0) m_ComboCount = 0;
	}
	if (m_BossHitPopTimer > 0) m_BossHitPopTimer--;
	if (m_KillChainTimer > 0)
	{
		m_KillChainTimer--;
		if (m_KillChainTimer == 0) m_KillChain = 0;
	}

	g_GameData.playTimeSec = m_FrameCount / 60;
	g_GameData.livesLeft = m_Lives;
	g_GameData.score = m_Score;
	g_GameData.grazeCount = m_GrazeCount;
	if (m_BossPhase > g_GameData.runStats.bossMaxPhaseReached)
		g_GameData.runStats.bossMaxPhaseReached = m_BossPhase;

	if (!m_Alive)
	{
		if (m_ClearDelayTimer == 0)
			SoundSynth::PlayExplosion();
		m_ClearDelayTimer++;
		m_Effect.Update();
		UpdateScreenShake();
		if (m_ClearDelayTimer >= 90)
		{
			g_GameData.isClear = false;
			g_GameData.isGameOver = true;
			g_GameData.runStats.maxKillChain = m_MaxKillChain;
			g_GameData.runStats.bombsUsed = m_BombsUsed;
			DisableSceneLighting();
			LeaveStage();
			BgmPlayer::Stop();
			return SceneType::Result;
		}
		return SceneType::None;
	}

	if (m_BossDefeated)
	{
		m_ClearDelayTimer++;
		m_Effect.Update();
		m_Bullets.Update(m_PlayerX, m_PlayerZ);
		UpdateScreenShake();
		if (m_ClearDelayTimer >= 130)
		{
			g_GameData.isClear = true;
			g_GameData.isGameOver = false;
			g_GameData.runStats.maxKillChain = m_MaxKillChain;
			g_GameData.runStats.bombsUsed = m_BombsUsed;
			g_GameData.runStats.clearedWithoutDamage =
				(m_Lives >= STAGE2_START_LIVES) && (m_BombsUsed == 0);
			g_GameData.runStats.noBombUsed = (m_BombsUsed == 0);
			DisableSceneLighting();
			LeaveStage();
			BgmPlayer::Stop();
			return SceneType::Result;
		}
		return SceneType::None;
	}

	UpdateMouseLook();
	UpdatePlayer();
	HandleBomb();
	HandleShooting();
	UpdateBoss();

	m_Bullets.Update(m_PlayerX, m_PlayerZ);
	CheckCollisions();
	m_Effect.Update();
	if (m_InvTimer > 0) m_InvTimer--;
	UpdateScreenShake();

	return SceneType::None;
}

void Stage2Scene::UpdateMouseLook()
{
	int mx = 0, my = 0;
	GetMousePoint(&mx, &my);

	if (m_PendingMouseRecenter)
	{
		RecenterMouse();
		m_PendingMouseRecenter = false;
		return;
	}

	const int dx = mx - m_MouseCenterX;
	const int dy = my - m_MouseCenterY;

	if (dx != 0 || dy != 0)
	{
		m_Yaw   += dx * MOUSE_SENSITIVITY;
		m_Pitch -= dy * MOUSE_SENSITIVITY;
		while (m_Yaw >  PI_F) m_Yaw -= TWO_PI_F;
		while (m_Yaw < -PI_F) m_Yaw += TWO_PI_F;
		m_Pitch = Clampf(m_Pitch, -PITCH_LIMIT, PITCH_LIMIT);
		RecenterMouse();
	}
}

void Stage2Scene::UpdatePlayer()
{
	const bool slow = CheckHitKey(KEY_INPUT_LSHIFT) || CheckHitKey(KEY_INPUT_RSHIFT);
	const float maxSpd = slow ? PLAYER_MAX_SPEED_SLOW : PLAYER_MAX_SPEED;

	float fx, fy, fz;
	float rx, ry, rz;
	ComputeForward(fx, fy, fz);
	ComputeRight(rx, ry, rz);

	float ix = 0.0f, iy = 0.0f, iz = 0.0f;
	if (CheckHitKey(KEY_INPUT_W)) { ix += fx; iy += fy; iz += fz; }
	if (CheckHitKey(KEY_INPUT_S)) { ix -= fx; iy -= fy; iz -= fz; }
	if (CheckHitKey(KEY_INPUT_A)) { ix -= rx;          iz -= rz; }
	if (CheckHitKey(KEY_INPUT_D)) { ix += rx;          iz += rz; }
	if (CheckHitKey(KEY_INPUT_E) || CheckHitKey(KEY_INPUT_R)) iy += 1.0f;
	if (CheckHitKey(KEY_INPUT_Q) || CheckHitKey(KEY_INPUT_F)) iy -= 1.0f;

	const float iLen = sqrtf(ix * ix + iy * iy + iz * iz);
	if (iLen > 0.0001f)
	{
		ix /= iLen; iy /= iLen; iz /= iLen;
		const float accel = PLAYER_ACCEL * (slow ? 0.75f : 1.0f);
		m_PlayerVX += ix * accel;
		m_PlayerVY += iy * accel;
		m_PlayerVZ += iz * accel;
	}

	m_PlayerVX *= PLAYER_DRAG;
	m_PlayerVY *= PLAYER_DRAG;
	m_PlayerVZ *= PLAYER_DRAG;

	float spdSq = m_PlayerVX * m_PlayerVX + m_PlayerVY * m_PlayerVY + m_PlayerVZ * m_PlayerVZ;
	if (spdSq > maxSpd * maxSpd)
	{
		const float s = maxSpd / sqrtf(spdSq);
		m_PlayerVX *= s;
		m_PlayerVY *= s;
		m_PlayerVZ *= s;
		spdSq = maxSpd * maxSpd;
	}
	m_PlayerSpeed = sqrtf(spdSq);

	m_PlayerX += m_PlayerVX;
	m_PlayerY += m_PlayerVY;
	m_PlayerZ += m_PlayerVZ;

	if (m_PlayerX >  PLAYER_BOUND_X) { m_PlayerX =  PLAYER_BOUND_X; m_PlayerVX *= -0.35f; }
	if (m_PlayerX < -PLAYER_BOUND_X) { m_PlayerX = -PLAYER_BOUND_X; m_PlayerVX *= -0.35f; }
	if (m_PlayerZ >  PLAYER_BOUND_Z) { m_PlayerZ =  PLAYER_BOUND_Z; m_PlayerVZ *= -0.35f; }
	if (m_PlayerZ < -PLAYER_BOUND_Z) { m_PlayerZ = -PLAYER_BOUND_Z; m_PlayerVZ *= -0.35f; }

	const float yMin = PLAYER_CENTER_Y - PLAYER_BOUND_Y;
	const float yMax = PLAYER_CENTER_Y + PLAYER_BOUND_Y;
	if (m_PlayerY > yMax) { m_PlayerY = yMax; m_PlayerVY *= -0.35f; }
	if (m_PlayerY < yMin) { m_PlayerY = yMin; m_PlayerVY *= -0.35f; }

	if (m_PlayerSpeed > 2.0f && (m_FrameCount & 1) == 0)
	{
		float bfx, bfy, bfz;
		ComputeForward(bfx, bfy, bfz);
		m_Effect.AddGrazeSpark(
			m_PlayerX - bfx * 16.0f,
			m_PlayerY - bfy * 16.0f,
			m_PlayerZ - bfz * 16.0f);
	}
}

void Stage2Scene::HandleShooting()
{
	if (m_FireCooldown > 0) m_FireCooldown--;

	const bool fireKey =
		CheckHitKey(KEY_INPUT_SPACE) ||
		CheckHitKey(KEY_INPUT_Z) ||
		(GetMouseInput() & MOUSE_INPUT_LEFT);

	if (!fireKey || m_FireCooldown > 0)
		return;

	const bool slow = CheckHitKey(KEY_INPUT_LSHIFT) || CheckHitKey(KEY_INPUT_RSHIFT);
	int interval = slow ? STAGE2_FIRE_INTERVAL_SLOW : STAGE2_FIRE_INTERVAL;
	if (m_PlayerSpeed > 3.5f) interval -= 1;
	if (interval < 2) interval = 2;
	m_FireCooldown = interval;

	float fx, fy, fz;
	ComputeForward(fx, fy, fz);
	float rx, ry, rz;
	ComputeRight(rx, ry, rz);

	const float bspd = STAGE2_PLAYER_BULLET_SPEED + m_PlayerSpeed * 1.2f;
	const unsigned int core = GetColor(220, 255, 255);
	const unsigned int main = GetColor(0, 255, 220);
	const unsigned int side = GetColor(120, 220, 255);

	const float muzzle = 20.0f;
	m_Bullets.AddPlayerBullet(
		m_PlayerX + fx * muzzle, m_PlayerY + fy * muzzle, m_PlayerZ + fz * muzzle,
		fx * bspd, fy * bspd, fz * bspd,
		STAGE2_PLAYER_BULLET_RADIUS * 1.45f, core, PlayerBulletKind::Normal, 0, 2);

	const float side1 = slow ? 5.0f : 11.0f;
	m_Bullets.AddPlayerBullet(
		m_PlayerX - rx * side1 + fx * 8.0f, m_PlayerY + fy * 14.0f, m_PlayerZ - rz * side1 + fz * 8.0f,
		fx * bspd * 0.98f, fy * bspd * 0.98f, fz * bspd * 0.98f,
		STAGE2_PLAYER_BULLET_RADIUS * 1.1f, main);
	m_Bullets.AddPlayerBullet(
		m_PlayerX + rx * side1 + fx * 8.0f, m_PlayerY + fy * 14.0f, m_PlayerZ + rz * side1 + fz * 8.0f,
		fx * bspd * 0.98f, fy * bspd * 0.98f, fz * bspd * 0.98f,
		STAGE2_PLAYER_BULLET_RADIUS * 1.1f, main);

	if (!slow)
	{
		const float side2 = 20.0f;
		m_Bullets.AddPlayerBullet(
			m_PlayerX - rx * side2, m_PlayerY, m_PlayerZ - rz * side2,
			(fx - rx * 0.05f) * bspd * 0.94f, fy * bspd * 0.94f, (fz - rz * 0.05f) * bspd * 0.94f,
			STAGE2_PLAYER_BULLET_RADIUS, side);
		m_Bullets.AddPlayerBullet(
			m_PlayerX + rx * side2, m_PlayerY, m_PlayerZ + rz * side2,
			(fx + rx * 0.05f) * bspd * 0.94f, fy * bspd * 0.94f, (fz + rz * 0.05f) * bspd * 0.94f,
			STAGE2_PLAYER_BULLET_RADIUS, side);
	}

	if ((m_FrameCount & 2) == 0)
		SoundSynth::PlayPlayerShoot();
}

void Stage2Scene::HandleBomb()
{
	const bool down = CheckHitKey(KEY_INPUT_X) || CheckHitKey(KEY_INPUT_B);
	if (down && !m_BombKeyHeld)
	{
		if (m_BombCount > 0)
		{
			m_BombCount--;
			m_BombsUsed++;
			m_InvTimer = 60;
			SoundSynth::PlayBomb();

			m_Bullets.ClearEnemyBullets();
			if (m_BossActive)
				TakeBossDamage(28);

			m_Effect.TriggerBombShockwave(m_PlayerX, m_PlayerY, m_PlayerZ);
			AddScreenShake(22, 12.0f);
			m_BombFlashTimer = 22;
			if (g_Options.hitStopEnabled) m_HitStopTimer = 3;
		}
	}
	m_BombKeyHeld = down;
}

void Stage2Scene::UpdateBoss()
{
	if (!m_BossActive) return;

	m_BossTimer++;
	m_BossPatternTimer++;

	// Drift target: orbit around player in 3D
	const int retargetEvery = (m_BossPhase == 0) ? 110 : 80;
	if ((m_BossTimer % retargetEvery) == 0)
	{
		const float ang = (float)GetRand(36000) / 100.0f * (PI_F / 180.0f);
		const float yang = FrandRange(-0.7f, 0.7f);
		const float r = FrandRange(BOSS_ORBIT_MIN_R, BOSS_ORBIT_MAX_R);
		m_BossTX = m_PlayerX + cosf(ang) * cosf(yang) * r;
		m_BossTY = m_PlayerY + sinf(yang) * r * 0.45f + 60.0f;
		m_BossTZ = m_PlayerZ + sinf(ang) * cosf(yang) * r;
	}

	float dx = m_BossTX - m_BossX;
	float dy = m_BossTY - m_BossY;
	float dz = m_BossTZ - m_BossZ;
	NormalizeVec3(dx, dy, dz);

	const float accel = 0.12f + 0.028f * (float)m_BossPhase;
	const float maxSpd = 1.8f + 0.55f * (float)m_BossPhase;
	m_BossVX = m_BossVX * 0.93f + dx * accel;
	m_BossVY = m_BossVY * 0.93f + dy * accel;
	m_BossVZ = m_BossVZ * 0.93f + dz * accel;
	float vsq = m_BossVX * m_BossVX + m_BossVY * m_BossVY + m_BossVZ * m_BossVZ;
	if (vsq > maxSpd * maxSpd)
	{
		float s = maxSpd / sqrtf(vsq);
		m_BossVX *= s; m_BossVY *= s; m_BossVZ *= s;
	}

	if (m_BossIntroTimer <= 0)
	{
		m_BossX += m_BossVX;
		m_BossY += m_BossVY;
		m_BossZ += m_BossVZ;
	}

	// During spell break boss keeps moving but doesn't fire
	if (m_SpellBreakTimer > 0)
	{
		m_SpellBreakTimer--;
		if (m_SpellBreakTimer == 0)
			StartNextPhase();
		return;
	}

	if (m_BossIntroTimer > 0)
		return;

	UpdateBossAttack();
}

void Stage2Scene::UpdateBossAttack()
{
	if (!m_Bullets.CanAddEnemyBullet()) return;

	const Difficulty d = g_GameData.difficulty;
	const float spdMul = Stage2BulletSpeedMul(d);

	// Aim direction from boss to player (used by some patterns)
	float ax = m_PlayerX - m_BossX;
	float ay = m_PlayerY - m_BossY;
	float az = m_PlayerZ - m_BossZ;
	NormalizeVec3(ax, ay, az);

	switch (m_BossPhase)
	{
	case 0:
	{
		// Aimed bursts every N frames + slow 8-way
		const int interval = Stage2FireIntervalScale(28, d);
		if ((m_BossPatternTimer % interval) == 0)
		{
			const float spd = 4.4f * spdMul;
			const float rad = 6.0f;
			for (int j = -1; j <= 1; j++)
			{
				const float ang = (float)j * 0.10f;
				const float cy = cosf(ang), sy = sinf(ang);
				const float nx = ax * cy - az * sy;
				const float nz = ax * sy + az * cy;
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					nx * spd, ay * spd, nz * spd, rad,
					BossBulletPhaseColor(0, j));
			}
		}
		const int ringInterval = Stage2FireIntervalScale(90, d);
		if ((m_BossPatternTimer % ringInterval) == 0)
		{
			const int n = 8;
			const float spd = 3.4f * spdMul;
			const float rad = 6.5f;
			for (int j = 0; j < n; j++)
			{
				const float a = (float)j / n * TWO_PI_F;
				const float vx = cosf(a) * spd;
				const float vz = sinf(a) * spd;
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					vx, 0.0f, vz, rad, BossBulletPhaseColor(0, 0));
			}
		}
		break;
	}
	case 1:
	{
		// Spiral pattern
		const int interval = Stage2FireIntervalScale(7, d);
		if ((m_BossPatternTimer % interval) == 0)
		{
			const int arms = 3;
			const float baseA = (float)m_BossPatternTimer * 0.18f;
			const float spd = 4.0f * spdMul;
			const float rad = 6.0f;
			for (int k = 0; k < arms; k++)
			{
				const float a = baseA + (float)k / arms * TWO_PI_F;
				const float py = sinf((float)m_BossPatternTimer * 0.05f) * 0.45f;
				const float vx = cosf(a) * spd;
				const float vz = sinf(a) * spd;
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					vx, py * spd, vz, rad, BossBulletPhaseColor(1, k));
			}
		}
		break;
	}
	case 2:
	{
		// 3D ring burst aimed up/down + horizontal ring
		const int interval = Stage2FireIntervalScale(50, d);
		if ((m_BossPatternTimer % interval) == 0)
		{
			const int n = 14;
			const float spd = 3.6f * spdMul;
			const float rad = 6.5f;
			const float yawOffset = (float)(m_BossPatternTimer / interval) * 0.21f;
			for (int j = 0; j < n; j++)
			{
				const float a = (float)j / n * TWO_PI_F + yawOffset;
				// horizontal ring
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * spd, 0.0f, sinf(a) * spd, rad,
					BossBulletPhaseColor(2, 0));
				// upward cone
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * spd * 0.65f, spd * 0.7f, sinf(a) * spd * 0.65f, rad,
					BossBulletPhaseColor(2, 1));
				// downward cone
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					cosf(a) * spd * 0.65f, -spd * 0.7f, sinf(a) * spd * 0.65f, rad,
					BossBulletPhaseColor(2, 0));
			}
		}
		// occasional aimed lance
		const int lance = Stage2FireIntervalScale(18, d);
		if ((m_BossPatternTimer % lance) == 0)
		{
			const float spd = 6.2f * spdMul;
			m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
				ax * spd, ay * spd, az * spd, 5.5f,
				BossBulletPhaseColor(2, 1));
		}
		break;
	}
	default:
	{
		// Final phase: dense omnidirectional + aimed bursts
		const int aimedInt = Stage2FireIntervalScale(14, d);
		if ((m_BossPatternTimer % aimedInt) == 0)
		{
			const float spd = 5.0f * spdMul;
			const float rad = 6.0f;
			for (int j = -2; j <= 2; j++)
			{
				const float ang = (float)j * 0.10f;
				const float cy = cosf(ang), sy = sinf(ang);
				const float nx = ax * cy - az * sy;
				const float nz = ax * sy + az * cy;
				m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
					nx * spd, ay * spd, nz * spd, rad,
					BossBulletPhaseColor(3, j));
			}
		}
		const int omniInt = Stage2FireIntervalScale(40, d);
		if ((m_BossPatternTimer % omniInt) == 0)
		{
			const int n = 16;
			const float spd = 3.2f * spdMul;
			const float rad = 6.5f;
			const float yawO = (float)(m_BossPatternTimer / omniInt) * 0.13f;
			for (int j = 0; j < n; j++)
			{
				const float a = (float)j / n * TWO_PI_F + yawO;
				for (int k = -1; k <= 1; k++)
				{
					const float py = (float)k * 0.5f;
					m_Bullets.AddEnemyBullet(m_BossX, m_BossY, m_BossZ,
						cosf(a) * spd, py * spd, sinf(a) * spd, rad,
						BossBulletPhaseColor(3, j + k));
				}
			}
		}
		break;
	}
	}
}

void Stage2Scene::StartNextPhase()
{
	m_BossPhase++;
	if (m_BossPhase >= STAGE2_BOSS_PHASES)
	{
		// Boss is defeated after final phase ends -> handled in TakeBossDamage
		m_BossPhase = STAGE2_BOSS_PHASES - 1;
		return;
	}
	m_BossMaxHp = GetBossRushPhaseHp(g_GameData.difficulty, m_BossPhase, m_BossPracticeMode);
	m_BossHp = m_BossMaxHp;
	m_BossDrawRadius = BossPhaseRadius(m_BossPhase);
	m_BossHitRadius  = m_BossDrawRadius * BOSS_HIT_RADIUS_MUL;
	m_BossPatternTimer = 0;
	m_SpellBannerTimer = 90;
	// Phase clear bomb bonus
	m_BombCount += BOMB_PHASE_CLEAR_BONUS;
	if (m_BombCount > BOMB_STOCK_MAX) m_BombCount = BOMB_STOCK_MAX;

	if (m_BossPhase > g_GameData.runStats.bossMaxPhaseReached)
		g_GameData.runStats.bossMaxPhaseReached = m_BossPhase;
}

void Stage2Scene::TakeBossDamage(int dmg)
{
	if (!m_BossActive) return;
	if (m_SpellBreakTimer > 0) return;
	if (m_BossIntroTimer > 0) return;

	m_BossHp -= dmg;
	m_BossHitFlash = 8;

	m_ComboCount++;
	m_ComboTimer = 60;
	const int hitScore = 20 + m_ComboCount * 8;
	m_Score += hitScore;
	m_BossHitPopScore = hitScore;
	m_BossHitPopTimer = 22;

	m_Effect.AddGrazeSpark(m_BossX, m_BossY, m_BossZ);
	AddScreenShake(6, 3.5f + (float)m_ComboCount * 0.15f);
	if (g_Options.hitStopEnabled && (m_FrameCount & 1) == 0)
		m_HitStopTimer = 1;

	if (m_BossHp <= 0)
	{
		m_Bullets.ClearEnemyBullets();
		m_Effect.AddExplosion(m_BossX, m_BossY, m_BossZ, BossPhaseColor(m_BossPhase), 12);
		AddScreenShake(22, 14.0f);
		if (g_Options.hitStopEnabled) m_HitStopTimer = 4;

		if (m_BossPhase + 1 >= STAGE2_BOSS_PHASES)
		{
			m_BossActive = false;
			m_BossDefeated = true;
			m_Score += 2000;
			SoundSynth::PlayExplosion();
		}
		else
		{
			m_SpellBreakTimer = GetBossRushSpellBreakFrames(g_GameData.difficulty);
			m_Score += SPELL_BONUS_SCORE;
			g_GameData.spellBonusCollected++;
			m_ComboCount = 0;
			SoundSynth::PlayExplosion();
		}
	}
}

void Stage2Scene::CheckCollisions()
{
	// Player bullets vs boss
	if (m_BossActive)
	{
		const int pbCount = m_Bullets.GetActivePlayerBulletCount();
		for (int li = pbCount - 1; li >= 0; li--)
		{
			const int idx = m_Bullets.GetActivePlayerBulletSlot(li);
			Bullet& b = m_Bullets.GetPlayerBulletsMutable()[idx];
			if (!b.active) continue;
			const float bvr = PlayerBulletCollisionRadius(b.radius);
			if (Collision::SphereVsSphere(b.x, b.y, b.z, bvr,
				m_BossX, m_BossY, m_BossZ, m_BossHitRadius))
			{
				if (m_SpellBreakTimer <= 0 && m_BossIntroTimer <= 0)
					TakeBossDamage(b.hitDamage);
				m_Bullets.ReleasePlayerBullet(idx);
			}
		}
	}

	// Enemy bullets vs player
	if (m_Alive && m_InvTimer <= 0)
	{
		const int ebCount = m_Bullets.GetActiveEnemyBulletCount();
		for (int li = ebCount - 1; li >= 0; li--)
		{
			const int idx = m_Bullets.GetActiveEnemyBulletSlot(li);
			Bullet& b = m_Bullets.GetEnemyBullets()[idx];
			if (!b.active) continue;
			const float br = EnemyBulletCollisionRadius(b.radius);
			if (Collision::SphereVsSphere(b.x, b.y, b.z, br,
				m_PlayerX, m_PlayerY, m_PlayerZ, STAGE2_PLAYER_RADIUS))
			{
				OnPlayerHit();
				m_Bullets.ReleaseEnemyBullet(idx);
				break;
			}
			else
			{
				const float gr = br + STAGE2_PLAYER_RADIUS + 18.0f;
				if (b.life == 0 &&
					Collision::SphereVsSphere(b.x, b.y, b.z, br,
						m_PlayerX, m_PlayerY, m_PlayerZ, gr))
				{
					m_GrazeCount++;
					m_Score += 5 + m_ComboCount * 2;
					b.life = 1;
					m_Effect.AddGrazeSpark(b.x, b.y, b.z);
					if ((m_FrameCount & 3) == 0)
						SoundSynth::PlayGraze();
				}
			}
		}
	}
}

void Stage2Scene::OnPlayerHit()
{
	m_DamageFlashTimer = 22;
	m_InvTimer = STAGE2_INV_FRAMES;
	AddScreenShake(14, 10.0f);
	SoundSynth::PlayExplosion();
	if (m_Lives > 0)
	{
		m_Lives--;
		m_Bullets.ClearEnemyBullets();
	}
	if (m_Lives <= 0)
		m_Alive = false;
}

void Stage2Scene::SetupCamera()
{
	SetCameraNearFar(1.0f, 4500.0f);

	float fx, fy, fz;
	ComputeForward(fx, fy, fz);

	const float camLerp = 0.14f;
	m_CamAnchorX += (m_PlayerX - m_CamAnchorX) * camLerp;
	m_CamAnchorY += (m_PlayerY - m_CamAnchorY) * camLerp;
	m_CamAnchorZ += (m_PlayerZ - m_CamAnchorZ) * camLerp;

	const float speedBoost = Clampf(m_PlayerSpeed * 6.0f, 0.0f, 36.0f);
	const float camDist   = 105.0f + speedBoost;
	const float camHeight = 38.0f + speedBoost * 0.25f;
	const float lookAhead = 210.0f + speedBoost * 1.5f;

	float shakeX = 0.0f, shakeY = 0.0f, shakeZ = 0.0f;
	if (m_ShakeTimer > 0)
	{
		const float t = (float)m_ShakeTimer;
		const float s = m_ShakeMag * (t / 14.0f);
		shakeX = cosf(t * 0.85f) * s;
		shakeY = sinf(t * 1.25f) * s * 0.6f;
		shakeZ = sinf(t * 0.65f) * s * 0.4f;
	}

	const float px = m_CamAnchorX + shakeX;
	const float py = m_CamAnchorY + shakeY;
	const float pz = m_CamAnchorZ + shakeZ;

	const float camPX = px - fx * camDist;
	const float camPY = py - fy * camDist + camHeight;
	const float camPZ = pz - fz * camDist;

	const float tgtX = px + fx * lookAhead;
	const float tgtY = py + fy * lookAhead;
	const float tgtZ = pz + fz * lookAhead;

	SetCameraPositionAndTargetAndUpVec(
		VGet(camPX, camPY, camPZ),
		VGet(tgtX, tgtY, tgtZ),
		VGet(0.0f, 1.0f, 0.0f));
}

void Stage2Scene::Draw()
{
	GameScreenSyncSize();

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	DrawBackground2D();

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);

	SetupCamera();
	if (m_LightingActive) SetUseLighting(TRUE);

	DrawSkybox3D();
	DrawArena3D();
	DrawPlayerShip3D();
	DrawBoss3D();

	SetUseLighting(FALSE);
	m_Bullets.DrawLit();
	m_Bullets.DrawEnemiesUnlit();
	m_Effect.Draw();
	if (m_LightingActive) SetUseLighting(TRUE);

	BeginScreenSpaceDraw();
	m_Effect.DrawScreenOverlay();
	DrawScreenFx();
	DrawSpeedFx();
	DrawCrosshair();
	DrawHud();
	DrawSpellBanner();
	if (m_Paused) DrawPauseOverlay();
}

void Stage2Scene::DrawBackground2D() const
{
	BeginScreenSpaceDraw();
	for (int y = 0; y < SCREEN_HEIGHT; y += 4)
	{
		const float t = (float)y / (float)SCREEN_HEIGHT;
		const int r = (int)(4 * (1.0f - t));
		const int g = (int)(2 * (1.0f - t));
		const int b = (int)(18 + 22 * (1.0f - t));
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(r, g, b));
	}
}

void Stage2Scene::DrawSkybox3D() const
{
	SetUseLighting(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_ADD, 70);
	for (int i = 0; i < STAGE2_SKYBOX_COUNT; i++)
	{
		const float ang = (float)i * 0.7853f + (float)i * 0.097f;
		const float h   = sinf((float)i * 0.31f);
		const float rr  = 2400.0f + (float)(i % 5) * 140.0f;
		const float x = m_PlayerX + cosf(ang) * rr;
		const float y = m_PlayerY + h * rr * 0.4f;
		const float z = m_PlayerZ + sinf(ang) * rr;
		const float r = 18.0f + (float)(i % 5) * 6.0f;
		const unsigned int col = ((i % 4) == 0)
			? GetColor(255, 200, 150)
			: GetColor(180, 200, 255);
		DrawSphere3D(VGet(x, y, z), r, STAGE2_SPHERE_SEG_LO, col, col, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2Scene::DrawArena3D() const
{
	SetUseLighting(FALSE);
	const float floorY = m_PlayerY - 220.0f;
	const float halfW = 900.0f;
	VECTOR fmin = VGet(-halfW, floorY - 4.0f, -halfW);
	VECTOR fmax = VGet( halfW, floorY,          halfW);
	DrawCube3D(fmin, fmax, GetColor(18, 26, 50), GetColor(40, 60, 100), TRUE);

	SetDrawBlendMode(DX_BLENDMODE_ADD, 70);
	const unsigned int gridCol = GetColor(0, 120, 200);
	const int step = 150;
	for (int i = -(int)halfW; i <= (int)halfW; i += step)
	{
		DrawLine3D(VGet((float)i, floorY + 1.0f, -halfW),
			VGet((float)i, floorY + 1.0f, halfW), gridCol);
		DrawLine3D(VGet(-halfW, floorY + 1.0f, (float)i),
			VGet( halfW, floorY + 1.0f, (float)i), gridCol);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2Scene::DrawPlayerShip3D() const
{
	if (!m_Alive) return;

	float fx, fy, fz;
	ComputeForward(fx, fy, fz);
	float rx, ry, rz;
	ComputeRight(rx, ry, rz);

	const bool blink = (m_InvTimer > 0) && ((m_FrameCount / 3) & 1);
	if (blink) return;

	const float r = STAGE2_PLAYER_SHIP_DRAW_R;
	const VECTOR center = VGet(m_PlayerX, m_PlayerY, m_PlayerZ);

	if (m_LightingActive) SetUseLighting(TRUE);
	DrawSphere3D(center, r, STAGE2_SPHERE_SEG_HI,
		GetColor(80, 220, 255), GetColor(220, 255, 255), TRUE);

	SetUseLighting(FALSE);
	// nose
	const VECTOR nose = VGet(m_PlayerX + fx * r * 1.4f,
	                         m_PlayerY + fy * r * 1.4f,
	                         m_PlayerZ + fz * r * 1.4f);
	DrawSphere3D(nose, r * 0.55f, STAGE2_SPHERE_SEG_LO,
		GetColor(180, 240, 255), GetColor(255, 255, 255), TRUE);
	// wings
	const VECTOR wingL = VGet(m_PlayerX - rx * r * 1.0f - fx * r * 0.3f,
	                          m_PlayerY,
	                          m_PlayerZ - rz * r * 1.0f - fz * r * 0.3f);
	const VECTOR wingR = VGet(m_PlayerX + rx * r * 1.0f - fx * r * 0.3f,
	                          m_PlayerY,
	                          m_PlayerZ + rz * r * 1.0f - fz * r * 0.3f);
	DrawSphere3D(wingL, r * 0.42f, STAGE2_SPHERE_SEG_LO,
		GetColor(50, 180, 230), GetColor(180, 230, 255), TRUE);
	DrawSphere3D(wingR, r * 0.42f, STAGE2_SPHERE_SEG_LO,
		GetColor(50, 180, 230), GetColor(180, 230, 255), TRUE);

	// engine glow (back) — stronger when moving fast
	SetDrawBlendMode(DX_BLENDMODE_ADD, (int)Clampf(100.0f + m_PlayerSpeed * 28.0f, 100.0f, 220.0f));
	const float exhaustScale = 0.55f + m_PlayerSpeed * 0.12f;
	const VECTOR exhaust = VGet(m_PlayerX - fx * r * 0.9f,
	                            m_PlayerY - fy * r * 0.9f,
	                            m_PlayerZ - fz * r * 0.9f);
	DrawSphere3D(exhaust, r * (exhaustScale + 0.12f * sinf((float)m_FrameCount * 0.5f)),
		STAGE2_SPHERE_SEG_LO,
		GetColor(0, 220, 255), GetColor(255, 255, 255), TRUE);
	if (m_PlayerSpeed > 3.0f)
	{
		DrawSphere3D(exhaust, r * (exhaustScale * 1.6f),
			STAGE2_SPHERE_SEG_LO,
			GetColor(80, 180, 255), GetColor(200, 240, 255), FALSE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2Scene::DrawBoss3D() const
{
	if (!m_BossActive) return;

	const unsigned int baseCol = m_BossHitFlash > 0 ? GetColor(255, 255, 255) : BossPhaseColor(m_BossPhase);
	const VECTOR pos = VGet(m_BossX, m_BossY, m_BossZ);

	if (m_LightingActive) SetUseLighting(TRUE);
	DrawSphere3D(pos, m_BossDrawRadius, STAGE2_SPHERE_SEG_HI,
		baseCol, GetColor(255, 255, 255), TRUE);

	SetUseLighting(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
	DrawSphere3D(pos, m_BossDrawRadius * 1.3f, STAGE2_SPHERE_SEG_LO,
		BossPhaseColor(m_BossPhase), GetColor(255, 255, 255), FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	if (m_BossIntroTimer > 0)
	{
		const float t = 1.0f - (float)m_BossIntroTimer / STAGE2_BOSS_INTRO_FRAMES;
		SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(180.0f * (1.0f - t)));
		DrawSphere3D(pos, m_BossDrawRadius * (1.4f + (1.0f - t) * 1.6f),
			STAGE2_SPHERE_SEG_LO, GetColor(255, 230, 200), GetColor(255, 255, 255), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void Stage2Scene::DrawCrosshair() const
{
	const int cx = SCREEN_WIDTH / 2;
	const int cy = SCREEN_HEIGHT / 2;

	bool lock = false;
	if (m_BossActive)
	{
		float fx, fy, fz;
		ComputeForward(fx, fy, fz);
		const float dx = m_BossX - m_PlayerX;
		const float dy = m_BossY - m_PlayerY;
		const float dz = m_BossZ - m_PlayerZ;
		const float dot = dx * fx + dy * fy + dz * fz;
		if (dot > 0.0f)
		{
			const float dlen2 = dx * dx + dy * dy + dz * dz;
			const float perp2 = dlen2 - dot * dot;
			const float rr = m_BossHitRadius + 30.0f;
			if (perp2 <= rr * rr) lock = true;
		}
	}

	const unsigned int col = lock ? GetColor(255, 80, 100) : GetColor(0, 255, 200);
	const int armOuter = 18;
	const int armInner = 6;
	const int thick = 2;

	DrawBox(cx - armOuter, cy - thick, cx - armInner, cy + thick, col, TRUE);
	DrawBox(cx + armInner, cy - thick, cx + armOuter, cy + thick, col, TRUE);
	DrawBox(cx - thick, cy - armOuter, cx + thick, cy - armInner, col, TRUE);
	DrawBox(cx - thick, cy + armInner, cx + thick, cy + armOuter, col, TRUE);
	DrawCircle(cx, cy, 2, col, TRUE);
}

void Stage2Scene::DrawHud() const
{
	BeginScreenSpaceDraw();

	const int marginX = 24;
	const int topY = 18;

	SetFontSize(22);
	DrawTextUtf8(marginX, topY, GetColor(0, 230, 255),
		u8"\uff33\uff34\uff21\uff27\uff25 \uff12\u3000\u30dc\u30b9\u6226\uff08\u5168\u65b9\u4f4d\uff09");

	SetFontSize(18);
	DrawFormatString(marginX, topY + 32, GetColor(220, 230, 255),
		u8"\u30b9\u30b3\u30a2  %08d", m_Score);
	DrawFormatString(marginX, topY + 56, GetColor(180, 220, 255),
		u8"\u6b8b\u6a5f    %d", m_Lives);
	DrawFormatString(marginX, topY + 80, GetColor(255, 200, 110),
		u8"\u30dc\u30e0    %d", m_BombCount);
	DrawFormatString(marginX, topY + 104, GetColor(180, 200, 220),
		u8"\u5f62\u614b    %d / %d", m_BossPhase + 1, STAGE2_BOSS_PHASES);

	if (m_KillChain >= 2)
	{
		DrawFormatString(marginX, topY + 128, GetColor(255, 220, 130),
			u8"\u9023\u6483    %dHIT", m_KillChain);
	}
	if (m_ComboCount >= 2)
	{
		DrawFormatString(marginX, topY + 152, GetColor(255, 180, 255),
			u8"\u30b3\u30f3\u30dc  %dHIT", m_ComboCount);
	}

	SetFontSize(15);
	const int rx = SCREEN_WIDTH - 320;
	DrawTextUtf8(rx, topY, GetColor(150, 180, 210),
		u8"\u30de\u30a6\u30b9       \uff1a\u8996\u70b9\uff08\u7167\u6e96\uff09");
	DrawTextUtf8(rx, topY + 20, GetColor(150, 180, 210),
		u8"\uff33\uff50\uff41\uff43\uff45\uff1a\u5c04\u6483\uff08\u9023\u5c04\uff09");
	DrawTextUtf8(rx, topY + 40, GetColor(150, 180, 210),
		u8"\uff37\uff21\uff53\uff24  \uff1a\u524d\u5f8c\u30fb\u5de6\u53f3\u79fb\u52d5");
	DrawTextUtf8(rx, topY + 60, GetColor(150, 180, 210),
		u8"\uff25\uff0f\uff22  \uff1a\u4e0a\u6607\u3000\uff33\uff0f\uff26  \uff1a\u4e0b\u964d");
	DrawTextUtf8(rx, topY + 80, GetColor(150, 180, 210),
		u8"\uff33\uff48\uff49\uff46\uff54\uff1a\u4f4e\u901f\u30fb\u7cbe\u5bc6");
	DrawTextUtf8(rx, topY + 100, GetColor(150, 180, 210),
		u8"\uff38\u30fb\uff22  \uff1a\u30dc\u30e0\u767a\u52d5");
	DrawTextUtf8(rx, topY + 120, GetColor(150, 180, 210),
		u8"\uff30\u30fbESC\uff1a\u30dd\u30fc\u30ba");

	// Boss HP bar (top center)
	if (m_BossActive)
	{
		const int barCx = SCREEN_WIDTH / 2;
		const int barW = SCREEN_WIDTH / 3;
		const int barTop = 24;
		const int barH = 14;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 130);
		DrawBox(barCx - barW / 2 - 2, barTop - 2, barCx + barW / 2 + 2, barTop + barH + 2,
			GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawBox(barCx - barW / 2, barTop, barCx + barW / 2, barTop + barH,
			GetColor(30, 32, 50), TRUE);
		int hpw = barW;
		if (m_BossMaxHp > 0) hpw = barW * m_BossHp / m_BossMaxHp;
		if (hpw < 0) hpw = 0;
		DrawBox(barCx - barW / 2, barTop, barCx - barW / 2 + hpw, barTop + barH,
			BossPhaseColor(m_BossPhase), TRUE);
		DrawBox(barCx - barW / 2, barTop, barCx + barW / 2, barTop + barH,
			GetColor(180, 200, 230), FALSE);

		SetFontSize(16);
		DrawTextUtf8Centered(barCx, barTop + barH + 4, GetColor(220, 230, 255),
			GetSpellCardName(0, m_BossPhase));
	}

	if (m_SpellBreakTimer > 0)
	{
		SetFontSize(28);
		DrawTextUtf8Centered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 40,
			GetColor(255, 230, 120),
			u8"\u30b9\u30da\u30eb\u30d6\u30ec\u30a4\u30af\uff01");
	}
	if (m_BossIntroTimer > 0)
	{
		SetFontSize(28);
		DrawTextUtf8Centered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 120,
			GetColor(255, 230, 120),
			u8"\u30dc\u30b9\u51fa\u73fe\uff01");
	}

	if (m_BossHitPopTimer > 0)
	{
		const int cx = SCREEN_WIDTH / 2 + 80;
		const int cy = SCREEN_HEIGHT / 2 - 40;
		int alpha = m_BossHitPopTimer * 8;
		if (alpha > 200) alpha = 200;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		SetFontSize(22 + (22 - m_BossHitPopTimer) / 3);
		char popBuf[32];
		sprintf_s(popBuf, sizeof(popBuf), "+%d", m_BossHitPopScore);
		DrawFormatString(cx, cy - (22 - m_BossHitPopTimer), GetColor(255, 240, 120), "%s", popBuf);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	SetFontSize(20);
}

void Stage2Scene::DrawSpellBanner() const
{
	if (m_SpellBannerTimer <= 0) return;
	const int t = m_SpellBannerTimer;
	int alpha = t * 3;
	if (alpha > 220) alpha = 220;
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
	SetFontSize(36);
	char buf[128];
	sprintf_s(buf, sizeof(buf),
		u8"\u7b2c%d\u5f62\u614b\u3000%s",
		m_BossPhase + 1, GetSpellCardName(0, m_BossPhase));
	DrawTextUtf8Centered(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 230,
		GetColor(255, 220, 120), buf);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2Scene::DrawSpeedFx() const
{
	if (m_PlayerSpeed < 2.5f) return;

	const int cx = SCREEN_WIDTH / 2;
	const int cy = SCREEN_HEIGHT / 2;
	const int alpha = (int)Clampf((m_PlayerSpeed - 2.0f) * 18.0f, 8.0f, 48.0f);
	SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

	float fx, fy, fz;
	ComputeForward(fx, fy, fz);
	const float len = sqrtf(fx * fx + fy * fy);
	if (len > 0.01f)
	{
		const float sx = -fx / len;
		const float sy = -fy / len;
		for (int i = 0; i < 8; i++)
		{
			const float t = (float)(i + 1) / 9.0f;
			const int x0 = cx + (int)(sx * 40.0f * t);
			const int y0 = cy + (int)(sy * 40.0f * t);
			const int x1 = cx + (int)(sx * (140.0f + m_PlayerSpeed * 18.0f) * t);
			const int y1 = cy + (int)(sy * (140.0f + m_PlayerSpeed * 18.0f) * t);
			DrawLine(x0, y0, x1, y1, GetColor(120, 220, 255));
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Stage2Scene::DrawScreenFx() const
{
	const int sw = SCREEN_WIDTH;
	const int sh = SCREEN_HEIGHT;

	if (m_DamageFlashTimer > 0)
	{
		int alpha = m_DamageFlashTimer * 6;
		if (alpha > 130) alpha = 130;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawBox(0, 0, sw, sh, GetColor(255, 60, 60), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	if (m_BombFlashTimer > 0)
	{
		int alpha = m_BombFlashTimer * 5;
		if (alpha > 120) alpha = 120;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		DrawBox(0, 0, sw, sh, GetColor(180, 240, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void Stage2Scene::DrawPauseOverlay() const
{
	BeginScreenSpaceDraw();
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	const int cx = SCREEN_WIDTH / 2;
	int cy = SCREEN_HEIGHT / 2 - 80;
	SetFontSize(42);
	DrawTextUtf8Centered(cx, cy, GetColor(255, 230, 120),
		u8"\uff30\uff21\uff35\uff33\uff25");
	cy += 70;

	static const char* items[] = {
		u8"\u30b2\u30fc\u30e0\u518d\u958b",
		u8"\u30bf\u30a4\u30c8\u30eb\u306b\u623b\u308b"
	};
	SetFontSize(26);
	for (int i = 0; i < 2; i++)
	{
		unsigned int col = (i == m_PauseMenuCursor) ? GetColor(255, 255, 0) : GetColor(200, 220, 240);
		char buf[96];
		sprintf_s(buf, sizeof(buf), "%s %s",
			(i == m_PauseMenuCursor) ? u8"\u25b6" : u8"\u3000", items[i]);
		DrawTextUtf8Centered(cx, cy + i * 40, col, buf);
	}
	SetFontSize(18);
	DrawTextUtf8Centered(cx, cy + 110, GetColor(180, 180, 200),
		u8"\u2191\u2193\uff1a\u9078\u629e\u3000Enter\uff1a\u6c7a\u5b9a\u3000\uff30\uff1a\u9589\u3058\u308b");
}

SceneType Stage2Scene::UpdatePauseMenu()
{
	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_UP))
		m_PauseMenuCursor = (m_PauseMenuCursor + 1) % 2;
	if (KeyHelper::IsMenuMoveTrigger(KEY_INPUT_DOWN))
		m_PauseMenuCursor = (m_PauseMenuCursor + 1) % 2;
	if (KeyHelper::IsTrigger(KEY_INPUT_P))
	{
		m_Paused = false;
		EnterStage();
		return SceneType::None;
	}
	if (KeyHelper::IsCancelTrigger())
	{
		m_Paused = false;
		EnterStage();
		return SceneType::None;
	}
	if (KeyHelper::IsConfirmTrigger())
	{
		if (m_PauseMenuCursor == 0)
		{
			m_Paused = false;
			EnterStage();
			return SceneType::None;
		}
		else
		{
			ExitToTitle();
			return SceneType::Title;
		}
	}
	return SceneType::None;
}

void Stage2Scene::ExitToTitle()
{
	DisableSceneLighting();
	LeaveStage();
	BgmPlayer::Stop();
}

void Stage2Scene::AddScreenShake(int frames, float magnitude)
{
	if (frames > m_ShakeTimer) m_ShakeTimer = frames;
	if (magnitude > m_ShakeMag) m_ShakeMag = magnitude;
}

void Stage2Scene::UpdateScreenShake()
{
	if (m_ShakeTimer > 0)
	{
		m_ShakeTimer--;
		if (m_ShakeTimer == 0) m_ShakeMag = 0.0f;
	}
}
