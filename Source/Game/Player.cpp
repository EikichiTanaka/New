#include "Game/Player.h"
#include "Game/SilhouetteDraw.h"
#include "Game/TouhouTheme.h"
#include "Common/ResourceManager.h"
#include "Game/CollisionDebug.h"
#include "Common/GameSession.h"
#include "Common/GameData.h"
#include "DxLib.h"
#include "Common/SoundSynth.h"
#include <algorithm>

void Player::Init(bool practiceMode, bool scoreAttackMode)
{
	m_X = PLAYER_START_X;
	m_Z = PLAYER_START_Z;
	m_FireCooldown = 0;
	m_InvTimer = 0;
	m_Alive = true;
	m_PracticeMode = practiceMode;
	m_ScoreAttackMode = scoreAttackMode;
	m_ChargeFrames = 0;
	m_WasShootHeld = false;

	m_Score = 0;
	m_Lives = (practiceMode || scoreAttackMode) ? 99 : START_LIVES;
	m_BombCount = START_BOMBS;
	m_GrazeCount = 0;
	m_FeverGauge = 0.0f;
	m_FeverTimer = 0;
	m_KillChain = 0;
	m_KillChainTimer = 0;
	m_GrazeRushCount = 0;
	m_GrazeRushTimer = 0;
	m_MaxKillChain = 0;
	m_FeverActivations = 0;
	m_BombsUsed = 0;
	m_FeverStartFlash = false;
	m_FeverReadyPing = false;
}

void Player::FireShots(BulletManager& bullets, bool fever)
{
	ShotType shot = g_Session.shotType;
	unsigned int main = GetColor(0, 255, 220);
	unsigned int side = GetColor(0, 180, 255);
	unsigned int core = GetColor(200, 255, 255);

	if (shot == ShotType::Pierce)
	{
		float spd = PBULLET_SPEED * (fever ? 1.15f : 1.08f);
		unsigned int pc = GetColor(255, 180, 255);
		bullets.AddPlayerBullet(m_X, PLAYER_Y, m_Z, 0, 0, spd, PBULLET_RADIUS * 1.35f, pc,
			PlayerBulletKind::Pierce, PIERCE_HIT_MAX);
		if (fever)
			bullets.AddPlayerBullet(m_X - 10, PLAYER_Y, m_Z, -0.4f, 0, spd, PBULLET_RADIUS * 1.1f, pc,
				PlayerBulletKind::Pierce, PIERCE_HIT_MAX);
	}
	else if (shot == ShotType::Homing)
	{
		unsigned int hc = GetColor(120, 255, 180);
		bullets.AddPlayerBullet(m_X - 8, PLAYER_Y, m_Z, -0.3f, 0, PBULLET_SPEED * 0.9f, PBULLET_RADIUS * 1.1f, hc,
			PlayerBulletKind::Homing, 0);
		bullets.AddPlayerBullet(m_X + 8, PLAYER_Y, m_Z, 0.3f, 0, PBULLET_SPEED * 0.9f, PBULLET_RADIUS * 1.1f, hc,
			PlayerBulletKind::Homing, 0);
		if (fever)
		{
			bullets.AddPlayerBullet(m_X, PLAYER_Y, m_Z, 0, 0, PBULLET_SPEED, PBULLET_RADIUS * 1.25f, hc,
				PlayerBulletKind::Homing, 0);
			bullets.AddPlayerBullet(m_X - 16, PLAYER_Y, m_Z, -0.8f, 0, PBULLET_SPEED * 0.85f, PBULLET_RADIUS, hc,
				PlayerBulletKind::Homing, 0);
			bullets.AddPlayerBullet(m_X + 16, PLAYER_Y, m_Z, 0.8f, 0, PBULLET_SPEED * 0.85f, PBULLET_RADIUS, hc,
				PlayerBulletKind::Homing, 0);
		}
	}
	else if (fever)
	{
		float fanSpeed = PBULLET_SPEED * 1.05f;
		unsigned int gold = GetColor(255, 215, 0);
		unsigned int amber = GetColor(255, 240, 120);
		bullets.AddPlayerBullet(m_X - 28, PLAYER_Y, m_Z, -4, 0, fanSpeed, PBULLET_RADIUS * 1.5f, gold);
		bullets.AddPlayerBullet(m_X + 28, PLAYER_Y, m_Z, 4, 0, fanSpeed, PBULLET_RADIUS * 1.5f, gold);
		bullets.AddPlayerBullet(m_X - 18, PLAYER_Y, m_Z, -2.2f, 0, PBULLET_SPEED, PBULLET_RADIUS * 1.5f, amber);
		bullets.AddPlayerBullet(m_X + 18, PLAYER_Y, m_Z, 2.2f, 0, PBULLET_SPEED, PBULLET_RADIUS * 1.5f, amber);
		bullets.AddPlayerBullet(m_X, PLAYER_Y, m_Z, 0, 0, PBULLET_SPEED * 1.2f, PBULLET_RADIUS * 2.0f, GetColor(255, 255, 200));
		bullets.AddPlayerBullet(m_X - 12, PLAYER_Y, m_Z, -1.2f, 0, PBULLET_SPEED * 1.1f, PBULLET_RADIUS * 1.3f, amber);
		bullets.AddPlayerBullet(m_X + 12, PLAYER_Y, m_Z, 1.2f, 0, PBULLET_SPEED * 1.1f, PBULLET_RADIUS * 1.3f, amber);
	}
	else
	{
		bullets.AddPlayerBullet(m_X, PLAYER_Y, m_Z, 0, 0, PBULLET_SPEED * 1.05f, PBULLET_RADIUS * 1.25f, core);
		bullets.AddPlayerBullet(m_X - 7, PLAYER_Y, m_Z, 0, 0, PBULLET_SPEED, PBULLET_RADIUS * 1.15f, main);
		bullets.AddPlayerBullet(m_X + 7, PLAYER_Y, m_Z, 0, 0, PBULLET_SPEED, PBULLET_RADIUS * 1.15f, main);
		bullets.AddPlayerBullet(m_X - 15, PLAYER_Y, m_Z, -1.1f, 0, PBULLET_SPEED * 0.98f, PBULLET_RADIUS, side);
		bullets.AddPlayerBullet(m_X + 15, PLAYER_Y, m_Z, 1.1f, 0, PBULLET_SPEED * 0.98f, PBULLET_RADIUS, side);
		bullets.AddPlayerBullet(m_X - 22, PLAYER_Y, m_Z, -2, 0, PBULLET_SPEED * 0.92f, PBULLET_RADIUS * 0.9f, side);
		bullets.AddPlayerBullet(m_X + 22, PLAYER_Y, m_Z, 2, 0, PBULLET_SPEED * 0.92f, PBULLET_RADIUS * 0.9f, side);
	}
}

// --- Update: 移動・入力・射撃更新 ---
void Player::Update(BulletManager& bullets)
{
	if (!m_Alive) return;

	// タイマーの更新
	if (m_InvTimer > 0) m_InvTimer--;
	if (m_FeverTimer > 0) m_FeverTimer--;
	if (m_FireCooldown > 0) m_FireCooldown--;
	if (m_KillChainTimer > 0)
	{
		m_KillChainTimer--;
		if (m_KillChainTimer == 0) m_KillChain = 0;
	}
	if (m_GrazeRushTimer > 0)
	{
		m_GrazeRushTimer--;
		if (m_GrazeRushTimer == 0) m_GrazeRushCount = 0;
	}

	// キー入力の受付
	bool moveLeft  = CheckHitKey(KEY_INPUT_LEFT)  != 0 || CheckHitKey(KEY_INPUT_A) != 0;
	bool moveRight = CheckHitKey(KEY_INPUT_RIGHT) != 0 || CheckHitKey(KEY_INPUT_D) != 0;
	bool moveUp    = CheckHitKey(KEY_INPUT_UP)    != 0 || CheckHitKey(KEY_INPUT_W) != 0;
	bool moveDown  = CheckHitKey(KEY_INPUT_DOWN)  != 0 || CheckHitKey(KEY_INPUT_S) != 0;
	
	// 低速移動（Shiftキー）
	bool slowMode  = CheckHitKey(KEY_INPUT_LSHIFT) != 0 || CheckHitKey(KEY_INPUT_RSHIFT) != 0;

	// 移動速度の決定
	float currentSpeed = slowMode ? PLAYER_SLOW_SPEED : PLAYER_SPEED;

	// XZ平面での移動
	if (moveLeft)  m_X -= currentSpeed;
	if (moveRight) m_X += currentSpeed;
	if (moveUp)    m_Z += currentSpeed;
	if (moveDown)  m_Z -= currentSpeed;

	// フィールド移動範囲のクランプ（壁めり込み防止）
	m_X = std::clamp(m_X, -FIELD_HALF_W + 20.0f, FIELD_HALF_W - 20.0f);
	m_Z = std::clamp(m_Z, -FIELD_HALF_D + 20.0f, FIELD_HALF_D - 20.0f);

	if ((m_PracticeMode || m_ScoreAttackMode) && CheckHitKey(KEY_INPUT_Q) != 0)
		CycleShotType();

	bool isShooting = CheckHitKey(KEY_INPUT_Z) != 0 || CheckHitKey(KEY_INPUT_SPACE) != 0;

	if (m_FeverGauge >= FEVER_GAUGE_READY && m_FeverTimer == 0 && !m_FeverReadyPing)
	{
		m_FeverReadyPing = true;
		SoundSynth::PlayFeverReady();
	}

	const bool moving = moveLeft || moveRight || moveUp || moveDown;
	const bool chargeBuild = isShooting && (slowMode || !moving);

	if (!isShooting && m_WasShootHeld && m_ChargeFrames >= CHARGE_MIN_FRAMES)
	{
		SoundSynth::PlayPlayerShoot();
		FireChargeShot(bullets);
		m_ChargeFrames = 0;
		m_FireCooldown = 12;
	}
	else if (chargeBuild)
	{
		if (m_ChargeFrames < CHARGE_MAX_FRAMES)
			m_ChargeFrames++;
	}
	else if (isShooting && m_FireCooldown == 0)
	{
		SoundSynth::PlayPlayerShoot();
		FireShots(bullets, IsFeverMode());
		m_FireCooldown = IsFeverMode() ? 1 : PLAYER_FIRE_RATE;
		if (m_ChargeFrames < CHARGE_MIN_FRAMES)
			m_ChargeFrames = 0;
	}
	else if (!isShooting && m_ChargeFrames > 0 && m_ChargeFrames < CHARGE_MIN_FRAMES)
	{
		m_ChargeFrames = 0;
	}

	m_WasShootHeld = isShooting;
}

void Player::CycleShotType()
{
	int t = (int)g_Session.shotType;
	t = (t + 1) % 3;
	g_Session.shotType = (ShotType)t;
}

void Player::FireChargeShot(BulletManager& bullets)
{
	float ratio = (float)m_ChargeFrames / (float)CHARGE_MAX_FRAMES;
	if (ratio > 1.0f) ratio = 1.0f;
	const float speed = PBULLET_SPEED * (1.25f + ratio * 0.95f);
	const float coreR = PBULLET_RADIUS * (2.0f + ratio * CHARGE_SHOT_RADIUS_MUL);
	const int pierce = PIERCE_HIT_MAX + CHARGE_SHOT_PIERCE_BONUS + (int)(ratio * 6.0f);
	const int damage = CHARGE_SHOT_DAMAGE_BASE + (int)(ratio * (float)CHARGE_SHOT_DAMAGE_SCALE);
	const unsigned int coreCol = GetColor(255, (int)(160 + ratio * 95), 255);
	const unsigned int sideCol = GetColor(255, (int)(120 + ratio * 80), 220);

	bullets.AddPlayerBullet(m_X, PLAYER_Y, m_Z, 0, 0, speed, coreR, coreCol,
		PlayerBulletKind::Charge, pierce, damage);

	const float sideR = coreR * 0.55f;
	const int sideDmg = (damage + 1) / 2;
	const float spread = 0.22f + ratio * 0.18f;
	bullets.AddPlayerBullet(m_X - 14.0f, PLAYER_Y, m_Z, -spread * speed, 0, speed * 0.92f,
		sideR, sideCol, PlayerBulletKind::Charge, pierce / 2 + 2, sideDmg);
	bullets.AddPlayerBullet(m_X + 14.0f, PLAYER_Y, m_Z, spread * speed, 0, speed * 0.92f,
		sideR, sideCol, PlayerBulletKind::Charge, pierce / 2 + 2, sideDmg);

	if (ratio >= 0.65f)
	{
		const float wideR = coreR * 0.75f;
		const int wideDmg = (damage * 2) / 3;
		bullets.AddPlayerBullet(m_X - 26.0f, PLAYER_Y, m_Z, -spread * 1.4f * speed, 0, speed * 0.85f,
			wideR, sideCol, PlayerBulletKind::Charge, pierce / 2, wideDmg);
		bullets.AddPlayerBullet(m_X + 26.0f, PLAYER_Y, m_Z, spread * 1.4f * speed, 0, speed * 0.85f,
			wideR, sideCol, PlayerBulletKind::Charge, pierce / 2, wideDmg);
	}
}

// --- Draw: 自機の描画 ---
void Player::Draw()
{
	if (!m_Alive) return;

	if (m_InvTimer > 0 && (m_InvTimer / 4) % 2 == 0)
	{
		return;
	}

	VECTOR pos = VGet(m_X, PLAYER_Y, m_Z);
	unsigned int bodyColor = IsFeverMode() ? GetColor(255, 220, 50) : GetColor(0, 230, 255);
	unsigned int glowColor = IsFeverMode() ? GetColor(255, 255, 200) : GetColor(255, 255, 255);

	bool slowMode = CheckHitKey(KEY_INPUT_LSHIFT) != 0 || CheckHitKey(KEY_INPUT_RSHIFT) != 0;

	if (PreferGameSprites() && ResourceManager::IsReady())
	{
		ResourceManager::DrawPlayer(m_X, PLAYER_Y, m_Z, IsFeverMode());
		if (CollisionDebug::IsVisible())
			DrawSphere3D(pos, PLAYER_COLLISION_RADIUS, 6, GetColor(255, 255, 255), GetColor(255, 255, 255), FALSE);
		return;
	}

	// [VISUAL_THEME] 巫女ビルボード（東方風）
	if (UseTouhouTheme() && TouhouTheme::IsReady())
	{
		if (VISUAL_RICH)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 55);
			VECTOR tp = VGet(m_X, PLAYER_Y, m_Z - 20.0f);
			DrawSphere3D(tp, PLAYER_DRAW_SIZE * 0.5f, 4, GetColor(255, 180, 220), GetColor(255, 180, 220), FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
		TouhouTheme::DrawPlayerBillboard(m_X, PLAYER_Y, m_Z, IsFeverMode());
		if (CollisionDebug::IsVisible())
			DrawSphere3D(pos, PLAYER_COLLISION_RADIUS, 6, GetColor(255, 255, 255), GetColor(255, 255, 255), FALSE);
		return;
	}

	// [VISUAL_RICH] 後方トレイル3連 + 加算ハロー（光感）
	if (VISUAL_RICH)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 70);
		for (int t = 0; t < 3; t++)
		{
			float ofs = 18.0f + t * 14.0f;
			float scl = 0.75f - t * 0.18f;
			VECTOR tp = VGet(m_X, PLAYER_Y, m_Z - ofs);
			DrawSphere3D(tp, PLAYER_DRAW_SIZE * scl, 6, bodyColor, bodyColor, FALSE);
		}
		SetDrawBlendMode(DX_BLENDMODE_ADD, RICH_PLAYER_HALO_ALPHA);
		float haloR = PLAYER_DRAW_SIZE * 1.7f + sinf((float)GetNowCount() / 120.0f) * 1.5f;
		DrawSphere3D(pos, haloR, 6, bodyColor, bodyColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
		VECTOR trailPos = VGet(m_X, PLAYER_Y, m_Z - 22.0f);
		DrawSphere3D(trailPos, PLAYER_DRAW_SIZE * 0.75f, 6, bodyColor, bodyColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// [VISUAL_STYLE] 小型メカ風シルエット / 従来の球
	if (UseSilhouetteStyle())
	{
		SilhouetteDraw::DrawPlayerMecha(m_X, PLAYER_Y, m_Z, bodyColor, glowColor, IsFeverMode());
	}
	else
	{
		DrawSphere3D(pos, PLAYER_DRAW_SIZE, 6, bodyColor, glowColor, FALSE);
	}

	if (m_ChargeFrames > 0)
	{
		float cr = (float)m_ChargeFrames / (float)CHARGE_MAX_FRAMES;
		if (cr > 1.0f) cr = 1.0f;
		SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(80 + cr * 120));
		float ringR = PLAYER_DRAW_SIZE * (1.1f + cr * 0.9f) + sinf((float)GetNowCount() / 70.0f) * 2.0f;
		DrawSphere3D(pos, ringR, 8, GetColor(255, 120, 255), GetColor(255, 200, 255), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (IsFeverMode())
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 55);
		float auraRadius = PLAYER_DRAW_SIZE + 5.0f + sinf((float)GetNowCount() / 120.0f) * 2.0f;
		DrawSphere3D(pos, auraRadius, 6, GetColor(255, 200, 80), GetColor(255, 220, 140), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (CollisionDebug::IsVisible())
	{
		DrawSphere3D(pos, PLAYER_COLLISION_RADIUS, 6, GetColor(255, 255, 255), GetColor(255, 255, 255), FALSE);
	}
}

// --- OnHit: 被弾時の処理 ---
void Player::OnHit()
{
	if (!m_Alive || m_InvTimer > 0) return;

	g_GameData.runStats.clearedWithoutDamage = false;
	if (!m_PracticeMode && !m_ScoreAttackMode)
		m_Lives--;
	m_InvTimer = PLAYER_INV_FRAMES;

	m_FeverGauge -= FEVER_GAUGE_HIT_PENALTY;
	if (m_FeverGauge < 0.0f) m_FeverGauge = 0.0f;
	m_FeverTimer = 0;
	m_KillChain = 0;
	m_KillChainTimer = 0;
	m_GrazeRushCount = 0;
	m_GrazeRushTimer = 0;

	if (!m_PracticeMode && !m_ScoreAttackMode && m_Lives <= 0)
		m_Alive = false;
}

void Player::AddFeverGauge(float amount)
{
	if (!m_Alive || m_FeverTimer > 0) return;

	m_FeverGauge += amount;
	if (m_FeverGauge > FEVER_GAUGE_MAX) m_FeverGauge = FEVER_GAUGE_MAX;
	TryTriggerFever();
}

void Player::TryTriggerFever()
{
	if (m_FeverGauge >= FEVER_GAUGE_MAX && m_FeverTimer == 0)
	{
		m_FeverGauge = 0.0f;
		m_FeverTimer = FEVER_DURATION_FRAMES;
		m_Score += FEVER_BONUS_SCORE;
		m_FeverActivations++;
		g_GameData.runStats.feverActivations = m_FeverActivations;
		m_FeverStartFlash = true;
		m_FeverReadyPing = false;
		m_KillChain = 0;
		m_GrazeRushCount = 0;
		SoundSynth::PlayFeverStart();
	}
}

bool Player::ConsumeFeverStartFlash()
{
	if (!m_FeverStartFlash) return false;
	m_FeverStartFlash = false;
	return true;
}

bool Player::ConsumeFeverReadyPing()
{
	if (!m_FeverReadyPing) return false;
	m_FeverReadyPing = false;
	return true;
}

void Player::OnEnemyKill()
{
	if (!m_Alive || m_FeverTimer > 0) return;

	AddFeverGauge(FEVER_GAUGE_PER_KILL);

	m_KillChain++;
	m_KillChainTimer = FEVER_KILL_CHAIN_WINDOW;
	if (m_KillChain > m_MaxKillChain) m_MaxKillChain = m_KillChain;
	if (g_GameData.runStats.maxKillChain < m_KillChain)
		g_GameData.runStats.maxKillChain = m_KillChain;
	if (m_KillChain >= FEVER_KILL_CHAIN_NEED)
	{
		AddFeverGauge(FEVER_GAUGE_CHAIN_BONUS);
		m_KillChain = 0;
	}
}

void Player::OnBossHit()
{
	if (!m_Alive || m_FeverTimer > 0) return;
	AddFeverGauge(FEVER_GAUGE_PER_BOSS_HIT);
}

void Player::AddGraze()
{
	if (!m_Alive) return;

	m_GrazeCount++;
	m_Score += SCORE_GRAZE;

	if (m_FeverTimer > 0) return;

	AddFeverGauge(FEVER_GAUGE_PER_GRAZE);

	m_GrazeRushCount++;
	m_GrazeRushTimer = FEVER_GRAZE_RUSH_WINDOW;
	if (m_GrazeRushCount >= FEVER_GRAZE_RUSH_NEED)
	{
		AddFeverGauge(FEVER_GAUGE_GRAZE_RUSH_BONUS);
		m_GrazeRushCount = 0;
	}
}

// --- AddScore: 得点の加算 ---
void Player::AddScore(int amount)
{
	float mult = g_GameData.scoreMultiplier;
	if (mult < 1.0f) mult = 1.0f;
	m_Score += (int)(amount * mult);
}

void Player::ForceInvincible(int frames)
{
	if (frames > m_InvTimer)
		m_InvTimer = frames;
}

bool Player::IsSlowMode() const
{
	return CheckHitKey(KEY_INPUT_LSHIFT) != 0 || CheckHitKey(KEY_INPUT_RSHIFT) != 0;
}

// --- TriggerBomb: ボム発動処理 ---
bool Player::TriggerBomb()
{
	if (!m_Alive || m_BombCount <= 0 || m_InvTimer > 100) return false;

	m_BombCount--;
	m_BombsUsed++;
	g_GameData.runStats.bombsUsed++;
	g_GameData.runStats.noBombUsed = false;
	m_InvTimer = 120;

	return true;
}
