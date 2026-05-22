#include "Scene/GameScene.h"
#include "GameConfig.h"
#include "Common/GameData.h"
#include "Common/GameSession.h"
#include "Common/SaveData.h"
#include "Common/SoundSynth.h"
#include "Common/GameOptions.h"
#include "Common/BgmPlayer.h"
#include "Common/KeyHelper.h"
#include "Common/UiDraw.h"
#include "Common/GameScreen.h"
#include "Game/Collision.h"
#include "DxLib.h"
#include <cmath>

// --- Init: ゲームシーン全体の初期化 ---
void GameScene::Init()
{
	m_FrameCount = 0;
	m_ClearDelayTimer = 0;

	m_Paused = false;
	m_HitStopTimer = 0;
	m_PrevWave = 1;
	m_WaveNoDamage = true;
	m_PrevSpellBreakTimer = 0;

	const bool practice = (g_Session.playMode == PlayMode::Practice);
	const bool scoreAttack = (g_Session.playMode == PlayMode::ScoreAttack);
	m_Player.Init(practice, scoreAttack);
	m_Bullets.Init();
	m_Enemies.Init(g_GameData.difficulty);
	m_Enemies.ConfigureStage(g_Session.stageStart, g_Session.playMode, g_Session.stageChapter);
	m_Effect.Init();
	InitItems();

	Difficulty diff = g_GameData.difficulty;
	g_GameData = {};
	g_GameData.difficulty = diff;
	g_GameData.livesLeft = m_Player.GetLives();
	g_GameData.runStats.noBombUsed = true;
	g_GameData.runStats.clearedWithoutDamage = true;
	g_GameData.scoreMultiplier = 1.0f;

	SoundSynth::Init();
	BgmPlayer::Stop();
	UpdateBackgroundBgm();

	DeleteLightHandleAll();
	m_DirLightHandle = CreateDirLightHandle(VGet(0.15f, -0.75f, 0.65f));
	SetLightDifColorHandle(m_DirLightHandle, GetColorF(0.85f, 0.88f, 0.95f, 1.0f));
	SetLightAmbColorHandle(m_DirLightHandle, GetColorF(0.40f, 0.44f, 0.55f, 1.0f));
	SetLightEnableHandle(m_DirLightHandle, TRUE);

	// ポイントライトは負荷が大きいので使わない（方向ライトのみで明るく見せる）
	m_PlayerLightHandle = -1;
	m_ForwardLightHandle = -1;

	m_BossHitFxCooldown = 0;

	m_CamX = PLAYER_START_X;
	m_CamY = PLAYER_Y + 92.0f;
	m_CamZ = PLAYER_START_Z - 235.0f;
	m_LookX = PLAYER_START_X;
	m_LookY = PLAYER_Y + 32.0f;
	m_LookZ = PLAYER_START_Z + 155.0f;
	m_ShakeTimer = 0;
	m_ShakeMag = 0.0f;
}

// --- Update: 1フレームの更新処理 ---
SceneType GameScene::Update()
{
	m_FrameCount++;

	if (m_ShakeTimer > 0)
	{
		m_ShakeTimer--;
		if (m_ShakeTimer == 0) m_ShakeMag = 0.0f;
	}

	if (KeyHelper::IsTrigger(KEY_INPUT_P))
		m_Paused = !m_Paused;

	if (m_Paused)
		return SceneType::None;

	if (m_HitStopTimer > 0)
	{
		m_HitStopTimer--;
		m_Effect.Update();
		return SceneType::None;
	}

	if (m_BossHitFxCooldown > 0) m_BossHitFxCooldown--;

	(void)m_Player.ConsumeFeverStartFlash();

	// ESC キーでゲーム強制終了（タイトルへ）
	if (CheckHitKey(KEY_INPUT_ESCAPE))
	{
		DisableSceneLighting();
		DeleteLightHandleAll();
		BgmPlayer::Stop();
		// SoundSynth::Final() は呼ばない（次プレイで効果音を作り直すのを防止）
		return SceneType::Title;
	}

	g_GameData.playTimeSec = m_FrameCount / 60;
	g_GameData.livesLeft = m_Player.GetLives();
	g_GameData.score = m_Player.GetScore();
	g_GameData.grazeCount = m_Player.GetGrazeCount();

	// --- 被弾・ゲームオーバーの終了シーケンス ---
	if (!m_Player.IsAlive())
	{
		if (m_ClearDelayTimer == 0)
		{
			SoundSynth::PlayExplosion();
		}
		m_ClearDelayTimer++;
		m_Effect.Update();

		if (m_ClearDelayTimer >= 90)
		{
			g_GameData.isClear = false;
			g_GameData.isGameOver = true;
			FinalizeRunStats();
			DisableSceneLighting();
			DeleteLightHandleAll();
			BgmPlayer::Stop();
			// SoundSynth::Final() は呼ばない
			return SceneType::Result;
		}
		return SceneType::None;
	}

	// --- ボス撃破・全ウェーブクリアの終了シーケンス ---
	if (m_Enemies.IsAllWavesComplete())
	{
		if (m_ClearDelayTimer == 0)
		{
			SoundSynth::PlayExplosion();
		}
		m_ClearDelayTimer++;
		m_Effect.Update();
		m_Bullets.Update(m_Player.GetX(), m_Player.GetZ());

		if (m_ClearDelayTimer >= 120)
		{
			g_GameData.isClear = true;
			g_GameData.isGameOver = false;
			FinalizeRunStats();
			DisableSceneLighting();
			DeleteLightHandleAll();
			BgmPlayer::Stop();
			// SoundSynth::Final() は呼ばない
			return SceneType::Result;
		}
		return SceneType::None;
	}

	// --- 通常のゲームプレイ更新 ---
	m_Player.Update(m_Bullets);

	// ボム入力判定 (X キー または B キー)
	bool isBombPressed = CheckHitKey(KEY_INPUT_X) != 0 || CheckHitKey(KEY_INPUT_B) != 0;
	if (isBombPressed)
	{
		ExecuteBomb();
	}

	int wave = m_Enemies.GetCurrentWave();
	if (wave != m_PrevWave)
	{
		if (m_WaveNoDamage)
			g_GameData.runStats.wavesNoDamage++;
		m_PrevWave = wave;
		m_WaveNoDamage = true;
	}

	if (m_Enemies.IsBossIntroActive())
		m_Player.ForceInvincible(BOSS_INTRO_FRAMES);

	int spellTimer = m_Enemies.GetSpellBreakTimer();
	if (spellTimer > 0 && m_PrevSpellBreakTimer <= 0)
		SpawnSpellBreakBonuses();
	m_PrevSpellBreakTimer = spellTimer;

	if (m_Enemies.GetBossPhase() > g_GameData.runStats.bossMaxPhaseReached)
		g_GameData.runStats.bossMaxPhaseReached = m_Enemies.GetBossPhase();

	m_Enemies.Update(m_Player.GetX(), m_Player.GetZ(), m_Bullets);
	m_Bullets.Update(m_Player.GetX(), m_Player.GetZ());
	m_Effect.Update();
	UpdateItems();
	CheckCollisions();

	BgmPlayer::Update();
	UpdateBackgroundBgm();

	g_GameData.livesLeft = m_Player.GetLives();
	g_GameData.score = m_Player.GetScore();
	g_GameData.grazeCount = m_Player.GetGrazeCount();

	return SceneType::None;
}

// --- SetupSceneLighting: DxLibライトで3D空間を照らす（方向ライトのみ） ---
void GameScene::SetupSceneLighting()
{
	SetUseLighting(TRUE);
	SetMaterialUseVertDifColor(TRUE);
	SetMaterialUseVertSpcColor(FALSE);
	SetGlobalAmbientLight(GetColorF(0.55f, 0.58f, 0.68f, 1.0f));

	// ポイントライトを廃止したので、毎フレームの位置更新は不要
}

// --- AddScreenShake: 撃破・被弾時のカメラシェイク ---
void GameScene::AddScreenShake(int frames, float magnitude)
{
	if (frames > m_ShakeTimer) m_ShakeTimer = frames;
	if (magnitude > m_ShakeMag) m_ShakeMag = magnitude;
}

// --- DisableSceneLighting: 2D描画・加算エフェクト用にライト無効化 ---
void GameScene::DisableSceneLighting()
{
	SetUseLighting(FALSE);
}

// --- Draw: 画面描画処理 ---
void GameScene::Draw()
{
	// 夜空色の背景（ライト無効のクリア色）
	SetBackgroundColor(24, 36, 58);
	SetUseLighting(FALSE);

	SetupCamera();

	SetupSceneLighting();
	DrawField();
	m_Player.Draw();
	m_Enemies.Draw();
	m_Bullets.DrawLit();
	DrawItems();
	DisableSceneLighting();
	m_Bullets.DrawEnemiesUnlit();
	BeginScreenSpaceDraw();
	m_Effect.Draw();

	DrawHud();
	DrawBossIntroOverlay();
	if (m_Paused) DrawPauseOverlay();
}

void GameScene::DrawPauseOverlay()
{
	BeginScreenSpaceDraw();
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 170);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetFontSize(48);
	DrawTextUtf8(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 24, GetColor(255, 255, 255), "ＰＡＵＳＥ");
	SetFontSize(20);
	DrawTextUtf8(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 40, GetColor(200, 200, 220), "Pキーで再開");
}

void GameScene::FinalizeRunStats()
{
	g_GameData.runStats.maxKillChain = m_Player.GetMaxKillChain();
	g_GameData.runStats.feverActivations = m_Player.GetFeverActivations();

	float bonusMul = 1.0f;
	if (g_GameData.isClear && g_GameData.runStats.noBombUsed)
		bonusMul *= SCORE_BONUS_NO_BOMB;
	if (g_GameData.isClear && g_GameData.runStats.clearedWithoutDamage)
		bonusMul *= SCORE_BONUS_NO_MISS;
	if (bonusMul > 1.0f)
	{
		g_GameData.score = (int)(g_GameData.score * bonusMul);
		g_GameData.scoreMultiplier = bonusMul;
	}

	if (g_Session.playMode == PlayMode::ScoreAttack && g_GameData.isClear)
		SaveDataTryUpdateScoreAttack(g_GameData.difficulty, g_GameData.score, g_GameData.playTimeSec);
	else if (g_Session.playMode != PlayMode::Practice)
		SaveDataTryUpdateHighScore(g_GameData.difficulty, g_GameData.score);

	g_GameData.rankLetter = ComputeRankLetter(
		g_GameData.score, g_GameData.isClear, g_GameData.grazeCount,
		g_GameData.runStats.maxKillChain, g_GameData.difficulty);
}

void GameScene::UpdateBackgroundBgm()
{
	BgmTrack track = BgmTrack::Stage1;
	if (m_Enemies.IsBossActive())
		track = BgmTrack::Boss;
	else if (m_Enemies.IsMidBossActive())
		track = BgmTrack::MidBoss;
	else if (m_Enemies.GetStageChapter() >= 1)
		track = BgmTrack::Stage2;
	BgmPlayer::Play(track);
}

void GameScene::SpawnSpellBreakBonuses()
{
	float bx = m_Enemies.GetBossX();
	float bz = m_Enemies.GetBossZ();
	for (int n = 0; n < 10; n++)
	{
		float ox = (float)(GetRand(200) - 100);
		float oz = (float)(GetRand(200) - 100);
		SpawnItem(bx + ox, bz + oz, 2);
	}
	g_GameData.scoreMultiplier += 0.08f;
}

void GameScene::DrawBossIntroOverlay() const
{
	if (!m_Enemies.IsBossIntroActive()) return;

	BeginScreenSpaceDraw();
	int t = m_Enemies.GetBossIntroTimer();
	float pulse = 0.5f + 0.5f * sinf((float)t * 0.12f);
	int a = (int)(120 + pulse * 80);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, a);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(40, 0, 60), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetFontSize(42);
	DrawTextUtf8(SCREEN_WIDTH / 2 - 200, 100, GetColor(255, 80, 120), "Spell Card");
	SetFontSize(28);
	const char* warn = (m_Enemies.GetStageChapter() >= 1)
		? "第２面 ボス戦開始"
		: "ボス戦開始";
	DrawTextUtf8(SCREEN_WIDTH / 2 - 140, 155, GetColor(255, 220, 180), warn);
	SetFontSize(20);
	DrawTextUtf8(SCREEN_WIDTH / 2 - 180, 200, GetColor(200, 200, 255), "無敵・弾消去中…");
}

// --- SetupCamera: 背面追従TPS（見下ろしではなく肩越し視点） ---
void GameScene::SetupCamera()
{
	SetCameraNearFar(1.0f, 3000.0f);

	float camDist = 235.0f;
	float camHeight = 90.0f;
	float lookAhead = 165.0f;
	const float camLerp = 0.16f;

	float px = m_Player.GetX();
	float py = m_Player.GetY();
	float pz = m_Player.GetZ();

	if (m_Enemies.IsBossIntroActive())
	{
		camDist = 175.0f;
		camHeight = 70.0f;
		lookAhead = 80.0f;
		px = m_Enemies.GetBossX() * 0.35f + px * 0.65f;
		pz = m_Enemies.GetBossZ() * 0.2f + pz * 0.8f - 40.0f;
	}
	else if (m_Enemies.IsBossActive())
	{
		camDist = 205.0f;
		lookAhead = 140.0f;
	}

	float destCamX = px;
	float destCamY = py + camHeight;
	float destCamZ = pz - camDist;

	float destLookX = px;
	float destLookY = py + 30.0f;
	float destLookZ = pz + lookAhead;

	m_CamX += (destCamX - m_CamX) * camLerp;
	m_CamY += (destCamY - m_CamY) * camLerp;
	m_CamZ += (destCamZ - m_CamZ) * camLerp;
	m_LookX += (destLookX - m_LookX) * camLerp;
	m_LookY += (destLookY - m_LookY) * camLerp;
	m_LookZ += (destLookZ - m_LookZ) * camLerp;

	float shakeX = 0.0f;
	float shakeY = 0.0f;
	float shakeZ = 0.0f;
	if (m_ShakeTimer > 0)
	{
		float t = (float)m_ShakeTimer;
		float s = m_ShakeMag * (t / 14.0f);
		shakeX = cosf(t * 0.85f) * s;
		shakeY = sinf(t * 1.25f) * s * 0.45f;
		shakeZ = sinf(t * 0.65f) * s * 0.35f;
	}

	VECTOR camPos = VGet(m_CamX + shakeX, m_CamY + shakeY, m_CamZ + shakeZ);
	VECTOR camTarget = VGet(m_LookX, m_LookY, m_LookZ);
	VECTOR camUp = VGet(0.0f, 1.0f, 0.0f);

	SetCameraPositionAndTargetAndUpVec(camPos, camTarget, camUp);
}

// --- DrawField: 3D床面（ライト適用）＋ネオングリッド ---
void GameScene::DrawField()
{
	float halfW = FIELD_WIDTH / 2.0f;
	float halfD = FIELD_DEPTH / 2.0f;

	// 床面（単色・ライトOFF）
	VECTOR floorMin = VGet(-halfW, -4.0f, -halfD);
	VECTOR floorMax = VGet(halfW, 0.0f, halfD);
	unsigned int floorCol = GetColor(32, 58, 88);
	DrawCube3D(floorMin, floorMax, floorCol, floorCol, FALSE);

	DisableSceneLighting();

	int gridColor = GetColor(0, 90, 50);
	float gridSpacing = (float)FIELD_GRID_SPACING;

	for (float z = -halfD; z <= halfD; z += gridSpacing)
		DrawLine3D(VGet(-halfW, 0.2f, z), VGet(halfW, 0.2f, z), gridColor);
	for (float x = -halfW; x <= halfW; x += gridSpacing)
		DrawLine3D(VGet(x, 0.2f, -halfD), VGet(x, 0.2f, halfD), gridColor);

	int borderColor = GetColor(0, 255, 120);
	DrawLine3D(VGet(-halfW, 0.2f, -halfD), VGet(halfW, 0.2f, -halfD), borderColor);
	DrawLine3D(VGet(-halfW, 0.2f, halfD), VGet(halfW, 0.2f, halfD), borderColor);
	DrawLine3D(VGet(-halfW, 0.2f, -halfD), VGet(-halfW, 0.2f, halfD), borderColor);
	DrawLine3D(VGet(halfW, 0.2f, -halfD), VGet(halfW, 0.2f, halfD), borderColor);

	SetUseLighting(TRUE);
	SetMaterialUseVertDifColor(TRUE);
}

// --- ExecuteBomb: ボム（スペルカード）発動 ---
void GameScene::ExecuteBomb()
{
	if (m_Player.TriggerBomb())
	{
		SoundSynth::PlayBomb();
		m_Effect.TriggerBombShockwave(m_Player.GetX(), PLAYER_Y, m_Player.GetZ());
		m_Bullets.ClearEnemyBullets();
		m_Enemies.DamageAllEnemies(18, m_Bullets);
		AddScreenShake(22, 14.0f);
		m_Effect.AddExplosion(m_Player.GetX(), PLAYER_Y, m_Player.GetZ(), GetColor(0, 255, 255), EXPLOSION_PARTICLE_COUNT);
	}
}

// --- CheckItemCollisions: 自機 vs アイテム回収 ---
void GameScene::CheckItemCollisions()
{
	const float px = m_Player.GetX();
	const float pz = m_Player.GetZ();

	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (!m_Items[i].active)
			continue;

		float dx = px - m_Items[i].x;
		float dz = pz - m_Items[i].z;
		float dist = sqrtf(dx * dx + dz * dz);

		bool pocCollect = pz > 100.0f;

		if (dist < 180.0f || pocCollect)
		{
			float pullSpeed = 4.5f;
			if (pocCollect) pullSpeed = 7.5f;

			if (dist > 0.01f)
			{
				m_Items[i].vx += (dx / dist) * pullSpeed * 0.15f;
				m_Items[i].vz += (dz / dist) * pullSpeed * 0.15f;
			}
		}

		if (dist < 18.0f)
		{
			m_Items[i].active = false;

			if (m_Items[i].type == 0)
			{
				m_Player.AddScore(500);
				m_Effect.AddItemSparkle(m_Items[i].x, PLAYER_Y, m_Items[i].z, GetColor(0, 255, 255));
			}
			else if (m_Items[i].type == 1)
			{
				m_Player.SetBombCount(m_Player.GetBombCount() + 1);
				m_Effect.AddItemSparkle(m_Items[i].x, PLAYER_Y, m_Items[i].z, GetColor(255, 200, 50));
			}
			else if (m_Items[i].type == 2)
			{
				m_Player.AddScore(SPELL_BONUS_SCORE);
				g_GameData.spellBonusCollected++;
				g_GameData.scoreMultiplier += 0.03f;
				m_Effect.AddItemSparkle(m_Items[i].x, PLAYER_Y, m_Items[i].z, GetColor(255, 120, 255));
			}

			SoundSynth::PlayPaddleBounce();
		}
	}
}

// --- CheckCollisions: 3D球体衝突およびグレイズ判定 ---
void GameScene::CheckCollisions()
{
	const int activePB = m_Bullets.GetActivePlayerBulletCount();
	const int activeEB = m_Bullets.GetActiveEnemyBulletCount();
	if (activePB == 0 && activeEB == 0)
	{
		CheckItemCollisions();
		return;
	}

	Bullet* pBullets = m_Bullets.GetPlayerBulletsMutable();

	Enemy* pEnemies = m_Enemies.GetEnemies();
	const int maxEnemies = m_Enemies.GetEnemyMax();

	Bullet* enemyBullets = m_Bullets.GetEnemyBullets();

	m_EnemyGrid.Clear();
	for (int j = 0; j < maxEnemies; j++)
	{
		if (pEnemies[j].IsActive())
			m_EnemyGrid.Insert(j, pEnemies[j].GetX(), pEnemies[j].GetZ());
	}

	m_EnemyBulletGrid.Clear();
	for (int li = 0; li < activeEB; li++)
	{
		const int bi = m_Bullets.GetActiveEnemyBulletSlot(li);
		m_EnemyBulletGrid.Insert(bi, enemyBullets[bi].x, enemyBullets[bi].z);
	}

	int hitSparkles = 0;
	int bossBulletReleases = 0;
	const float playerQueryRadius = GRAZE_RADIUS + 90.0f;
	const float bulletEnemyQueryPad = ENEMY_DRAW_SIZE + 85.0f;

	const bool bossActive = m_Enemies.IsBossActive() && !m_Enemies.IsSpellBreakActive();
	const bool midBossActive = m_Enemies.IsMidBossActive();
	const float bossX = bossActive ? m_Enemies.GetBossX() : 0.0f;
	const float bossZ = bossActive ? m_Enemies.GetBossZ() : 0.0f;
	const float bossR = bossActive ? m_Enemies.GetBossRadius() : 0.0f;
	const float midX = midBossActive ? m_Enemies.GetMidBossX() : 0.0f;
	const float midZ = midBossActive ? m_Enemies.GetMidBossZ() : 0.0f;
	const float midR = midBossActive ? m_Enemies.GetMidBossRadius() : 0.0f;

	auto releasePlayerBulletCapped = [&](int slotIndex)
	{
		if (bossBulletReleases >= MAX_BOSS_BULLET_RELEASES_FRAME)
			return;
		m_Bullets.ReleasePlayerBullet(slotIndex);
		bossBulletReleases++;
	};

	// 自機弾 vs 敵（ボス/中ボスは近傍のみ・雑魚は空間分割）
	for (int li = 0; li < activePB; li++)
	{
		const int i = m_Bullets.GetActivePlayerBulletSlot(li);
		Bullet& pb = pBullets[i];
		const float bx = pb.x;
		const float bz = pb.z;
		const float br = pb.radius;
		bool hitResolved = false;

		if (bossActive && pb.life <= 0 &&
			Collision::SphereXZBroadPhase(bx, bz, br, bossX, bossZ, bossR))
		{
			if (Collision::SphereVsSphere(
				bx, PLAYER_Y, bz, br,
				bossX, PLAYER_Y, bossZ, bossR))
			{
				const bool keep = (pb.kind == PlayerBulletKind::Pierce && pb.pierceLeft > 0);
				if (keep) pb.pierceLeft--;

				pb.life = BOSS_BULLET_COLLISION_SKIP;

				if (m_Enemies.TryApplyBossBulletHit(1, m_Bullets))
				{
					if (!keep) releasePlayerBulletCapped(i);
					m_Player.AddScore(30);
					m_Player.OnBossHit();
					if (m_BossHitFxCooldown <= 0)
					{
						AddScreenShake(4, 5.0f);
						m_BossHitFxCooldown = BOSS_HIT_FX_COOLDOWN;
					}
					if (hitSparkles < MAX_HIT_SPARKLES_FRAME)
					{
						m_Effect.AddItemSparkle(bx, PLAYER_Y, bz, GetColor(255, 255, 100));
						hitSparkles++;
					}
				}
				else if (!keep)
				{
					releasePlayerBulletCapped(i);
				}
				continue;
			}
		}

		if (midBossActive && pb.life <= 0 &&
			Collision::SphereXZBroadPhase(bx, bz, br, midX, midZ, midR))
		{
			if (Collision::SphereVsSphere(
				bx, PLAYER_Y, bz, br,
				midX, PLAYER_Y, midZ, midR))
			{
				const bool keep = (pb.kind == PlayerBulletKind::Pierce && pb.pierceLeft > 0);
				if (keep) pb.pierceLeft--;

				pb.life = BOSS_BULLET_COLLISION_SKIP;

				if (m_Enemies.TryApplyMidBossBulletHit(1, m_Bullets))
				{
					if (!keep) releasePlayerBulletCapped(i);
					m_Player.AddScore(25);
					if (m_BossHitFxCooldown <= 0)
					{
						AddScreenShake(3, 4.0f);
						m_BossHitFxCooldown = BOSS_HIT_FX_COOLDOWN;
					}
				}
				else if (!keep)
				{
					releasePlayerBulletCapped(i);
				}
				continue;
			}
		}

		m_EnemyGrid.ForEachNear(bx, bz, br + bulletEnemyQueryPad, [&](int j)
		{
			if (hitResolved || !pEnemies[j].IsActive())
				return;

			const float ex = pEnemies[j].GetX();
			const float ez = pEnemies[j].GetZ();
			if (!Collision::SphereVsSphere(
				bx, PLAYER_Y, bz, br,
				ex, PLAYER_Y, ez, pEnemies[j].GetRadius()))
				return;

			hitResolved = true;
			const bool keep = (pb.kind == PlayerBulletKind::Pierce && pb.pierceLeft > 0);
			if (keep) pb.pierceLeft--;

			const EnemyType killedType = pEnemies[j].GetType();
			if (pEnemies[j].TryApplyBulletDamage(1))
			{
				if (!keep) m_Bullets.ReleasePlayerBullet(i);

				if (!pEnemies[j].IsActive())
				{
					g_GameData.runStats.enemiesDestroyed++;
					unsigned int exColor = (killedType == EnemyType::Spinner)
						? GetColor(255, 100, 0) : GetColor(255, 50, 50);
					m_Effect.AddExplosion(ex, PLAYER_Y, ez, exColor, EXPLOSION_PARTICLE_COUNT);
					AddScreenShake(10, 6.0f);
					if (g_Options.hitStopEnabled)
						m_HitStopTimer = HITSTOP_FRAMES;
					if (killedType == EnemyType::Splitter)
						m_Enemies.SpawnSplitterChildren(ex, ez);
					SoundSynth::PlayExplosion();
					m_Player.AddScore(SCORE_PER_ENEMY);
					m_Player.OnEnemyKill();

					const int rnd = GetRand(99);
					if (rnd < 78)      SpawnItem(ex, ez, 0);
					else if (rnd < 93) SpawnItem(ex, ez, 1);
				}
				else if (hitSparkles < MAX_HIT_SPARKLES_FRAME)
				{
					m_Effect.AddItemSparkle(bx, PLAYER_Y, bz, GetColor(255, 120, 120));
					hitSparkles++;
				}
			}
			else if (!keep)
			{
				m_Bullets.ReleasePlayerBullet(i);
			}
		});
	}

	// B. 敵弾 vs 自機（空間分割で自機近傍の弾のみ）
	const float px = m_Player.GetX();
	const float pz = m_Player.GetZ();
	const float pr = m_Player.GetRadius();

	m_EnemyBulletGrid.ForEachNear(px, pz, playerQueryRadius, [&](int i)
	{
		Bullet& eb = enemyBullets[i];
		if (!eb.active)
			return;

		if (m_Effect.IsBombActive())
		{
			const float bRadius = m_Effect.GetBombShockwaveRadius();
			if (Collision::SphereVsSphere(
				eb.x, PLAYER_Y, eb.z, eb.radius,
				px, PLAYER_Y, pz, bRadius))
			{
				m_Bullets.ReleaseEnemyBullet(i);
				if (hitSparkles < MAX_HIT_SPARKLES_FRAME)
				{
					m_Effect.AddItemSparkle(eb.x, PLAYER_Y, eb.z, GetColor(100, 255, 255));
					hitSparkles++;
				}
				return;
			}
		}

		if (Collision::SphereVsSphere(
			eb.x, PLAYER_Y, eb.z, eb.radius,
			px, PLAYER_Y, pz, pr))
		{
			if (!m_Player.IsInvincible())
			{
				m_Bullets.ReleaseEnemyBullet(i);
				m_Player.OnHit();
				m_WaveNoDamage = false;
				SoundSynth::PlayExplosion();
				m_Effect.AddExplosion(px, PLAYER_Y, pz, GetColor(0, 200, 255), EXPLOSION_PARTICLE_COUNT);
				AddScreenShake(14, 9.0f);
			}
			return;
		}

		if (eb.life != -99)
		{
			if (Collision::SphereVsSphere(
				eb.x, PLAYER_Y, eb.z, eb.radius,
				px, PLAYER_Y, pz, GRAZE_RADIUS))
			{
				eb.life = -99;
				m_Player.AddGraze();
				m_Effect.AddGrazeSpark(px, PLAYER_Y, pz);
				AddScreenShake(3, 2.5f);
				SoundSynth::PlayGraze();
			}
		}
	});

	CheckItemCollisions();
}

void GameScene::InitItems()
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		m_Items[i].active = false;
	}
}

void GameScene::SpawnItem(float x, float z, int type)
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (!m_Items[i].active)
		{
			m_Items[i].x = x;
			m_Items[i].y = PLAYER_Y;
			m_Items[i].z = z;
			m_Items[i].vx = (float)(GetRand(40) - 20) / 10.0f;
			m_Items[i].vz = 1.0f + (float)GetRand(20) / 10.0f;
			m_Items[i].type = type;
			m_Items[i].active = true;
			break;
		}
	}
}

void GameScene::UpdateItems()
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (m_Items[i].active)
		{
			m_Items[i].x += m_Items[i].vx;
			m_Items[i].z += m_Items[i].vz;
			m_Items[i].vx *= 0.96f;
			m_Items[i].vz *= 0.96f;

			if (m_Items[i].z < -FIELD_HALF_D - 50.0f)
			{
				m_Items[i].active = false;
			}
		}
	}
}

void GameScene::DrawItems() const
{
	for (int i = 0; i < ITEM_MAX; i++)
	{
		if (m_Items[i].active)
		{
			float rotSize = 6.0f;
			VECTOR center = VGet(m_Items[i].x, PLAYER_Y, m_Items[i].z);

			if (m_Items[i].type == 0)
			{
				VECTOR minPos = VGet(center.x - rotSize, center.y - rotSize, center.z - rotSize);
				VECTOR maxPos = VGet(center.x + rotSize, center.y + rotSize, center.z + rotSize);
				DrawCube3D(minPos, maxPos, GetColor(0, 200, 255), GetColor(255, 255, 255), FALSE);
			}
			else if (m_Items[i].type == 2)
			{
				DrawSphere3D(center, rotSize + 2.0f, 6, GetColor(255, 80, 255), GetColor(255, 220, 255), FALSE);
			}
			else
			{
				DrawSphere3D(center, rotSize + 1.0f, 6, GetColor(255, 180, 0), GetColor(255, 255, 200), FALSE);
			}
		}
	}
}

// --- DrawHud: ネオンカラーのアーケードHUD描画（日本語ラベル付き） ---
// 最適化: SetFontSize 呼び出しを集約、固定ラベルは DrawTextUtf8、
//        ライフ/ボム記号は DrawTriangle / DrawCircle で描画
void GameScene::DrawHud()
{
	GameScreenSyncSize();
	BeginScreenSpaceDraw();

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 110);
	DrawBox(0, 0, SCREEN_WIDTH, 140, GetColor(0, 8, 22), TRUE);
	DrawBox(0, SCREEN_HEIGHT - 70, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 8, 22), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// === サイズ18の固定ラベルをまとめて描画 ===
	SetFontSize(18);
	DrawTextUtf8(30, 18, GetColor(255, 255, 255), "ＳＣＯＲＥ ／ スコア");
	DrawTextUtf8(30, 80, GetColor(255, 120, 120), "ＬＩＦＥ ／ 残機");
	DrawTextUtf8(SCREEN_WIDTH - 220, 18, GetColor(200, 200, 200), "ＢＯＭＢ ／ ボム");
	DrawTextUtf8(SCREEN_WIDTH - 220, 80, GetColor(255, 200, 0), "ＧＲＡＺＥ ／ かすり");
	DrawTextUtf8(SCREEN_WIDTH / 2 - 90, 18, GetColor(200, 200, 200), "ＷＡＶＥ ／ 進行段階");

	// === ライフ表示: 赤い菱形（DrawTriangle で軽量描画） ===
	int lives = m_Player.GetLives();
	for (int i = 0; i < START_LIVES; i++)
	{
		int cx = 30 + i * 28 + 11;
		int cy = 102 + 14;
		unsigned int col = (i < lives) ? GetColor(255, 60, 100) : GetColor(80, 80, 80);
		DrawTriangle(cx, cy - 11, cx - 10, cy, cx + 10, cy, col, TRUE);
		DrawTriangle(cx, cy + 11, cx - 10, cy, cx + 10, cy, col, TRUE);
	}

	// === ボムストック表示: 円（DrawCircle で軽量描画） ===
	int bombs = m_Player.GetBombCount();
	for (int i = 0; i < 5; i++)
	{
		int cx = SCREEN_WIDTH - 220 + i * 30 + 12;
		int cy = 40 + 14;
		unsigned int col = (i < bombs) ? GetColor(0, 255, 200) : GetColor(50, 50, 50);
		DrawCircle(cx, cy, 10, col, TRUE);
	}

	// === サイズ28-30のスコア/数値をまとめて ===
	SetFontSize(28);
	DrawFormatString(30, 40, GetColor(0, 255, 255), "%08d", m_Player.GetScore());

	SetFontSize(26);
	DrawFormatString(SCREEN_WIDTH - 220, 102, GetColor(255, 220, 0), "%d 回", m_Player.GetGrazeCount());

	// === ウェーブ番号 ===
	int wave = m_Enemies.GetCurrentWave();
	SetFontSize(30);
	DrawFormatString(SCREEN_WIDTH / 2 - 50, 40, GetColor(255, 255, 255), "%d ／ %d", wave, MAX_WAVES);

	// === ウェーブ愛称 ===
	static const char* const waveTitlesJp[] = {
		"[ 弾幕開幕　紅蓮の雨 ]",
		"[ スペル　螺旋ノ調 ]",
		"[ 鬼弾幕　鉄壁陣形 ]",
		"[ 狂騒曲　弾幕カグラ風 ]",
		"[ 終章　禁忌の弾幕祭 ]"
	};
	int wi = wave - 1;
	if (wi < 0) wi = 0;
	if (wi > 4) wi = 4;
	SetFontSize(18);
	DrawTextUtf8(SCREEN_WIDTH / 2 - 120, 80, GetColor(255, 180, 0), waveTitlesJp[wi]);

	// === フィーバーゲージ/メーター（下端中央） ===
	int meterW = 380;
	int meterH = 12;
	int mx = SCREEN_WIDTH / 2 - meterW / 2;
	int my = SCREEN_HEIGHT - 38;

	DrawBox(mx - 2, my - 2, mx + meterW + 2, my + meterH + 2, GetColor(40, 40, 60), TRUE);

	bool feverOn = m_Player.IsFeverMode();
	if (feverOn)
	{
		float fRatio = (float)m_Player.GetFeverTimer() / (float)Player::GetFeverDurationMax();
		int fWidth = (int)(meterW * fRatio);
		unsigned int fColor = ((m_FrameCount / 5) % 2 == 0) ? GetColor(255, 215, 0) : GetColor(255, 255, 100);
		DrawBox(mx, my, mx + fWidth, my + meterH, fColor, TRUE);

		SetFontSize(22);
		DrawTextUtf8(SCREEN_WIDTH / 2 - 190, my - 28, fColor, "！！  フィーバータイム　超火力弾幕  ！！");
	}
	else
	{
		float gRatio = m_Player.GetFeverGauge() / FEVER_GAUGE_MAX;
		int gWidth = (int)(meterW * gRatio);
		unsigned int gaugeCol = m_Player.IsFeverGaugeReady() ? GetColor(255, 220, 80) : GetColor(0, 220, 255);
		DrawBox(mx, my, mx + gWidth, my + meterH, gaugeCol, TRUE);

		SetFontSize(18);
		DrawFormatString(SCREEN_WIDTH / 2 - 220, my - 24, gaugeCol,
			"ＦＥＶＥＲ 撃破%d/%d  かすりラッシュ可", m_Player.GetKillChain(), FEVER_KILL_CHAIN_NEED);
	}

	// === ボスHPバー ===
	if (m_Enemies.IsBossActive())
	{
		int bossHp = m_Enemies.GetBossHp();
		int bossMaxHp = m_Enemies.GetBossMaxHp();
		float bossRatio = bossMaxHp > 0 ? (float)bossHp / (float)bossMaxHp : 0.0f;
		int bossBarW = 400;
		int bx = SCREEN_WIDTH / 2 - bossBarW / 2;
		int by = 118;
		DrawBox(bx - 2, by - 2, bx + bossBarW + 2, by + 14, GetColor(50, 0, 10), TRUE);
		DrawBox(bx, by, bx + (int)(bossBarW * bossRatio), by + 10, GetColor(255, 0, 100), TRUE);
		SetFontSize(18);
		DrawTextUtf8(bx, by + 14, GetColor(255, 200, 100), "ＢＯＳＳ");
	}

	// === モード/倍率/スペル/チャージ表示 ===
	// 同サイズが連続するのでまとめて切替
	bool practice = m_Player.IsPracticeMode();
	bool scoreAtk = m_Player.IsScoreAttackMode();
	bool showMul = (g_GameData.scoreMultiplier > 1.01f);
	bool showSpellPickup = (g_GameData.spellBonusCollected > 0);
	bool spellBreak = m_Enemies.IsSpellBreakActive();
	int charge = m_Player.GetChargeFrames();

	if (practice || scoreAtk || showMul)
	{
		SetFontSize(18);
		if (practice)
			DrawTextUtf8(30, SCREEN_HEIGHT - 62, GetColor(120, 255, 180), "【練習モード】");
		if (scoreAtk)
			DrawTextUtf8(30, SCREEN_HEIGHT - 88, GetColor(255, 220, 120), "【スコアアタック】");
		if (showMul)
			DrawFormatString(SCREEN_WIDTH - 240, 108, GetColor(255, 180, 255),
				"得点倍率 x%.2f", g_GameData.scoreMultiplier);
	}
	if (showSpellPickup)
	{
		SetFontSize(16);
		DrawFormatString(SCREEN_WIDTH - 240, 130, GetColor(255, 140, 220),
			"スペル拾い %d", g_GameData.spellBonusCollected);
	}

	if (charge > 0)
	{
		float cr = (float)charge / (float)CHARGE_MAX_FRAMES;
		if (cr > 1.0f) cr = 1.0f;
		int cw = (int)(120 * cr);
		DrawBox(SCREEN_WIDTH / 2 - 62, SCREEN_HEIGHT - 58, SCREEN_WIDTH / 2 - 62 + cw, SCREEN_HEIGHT - 46,
			GetColor(255, 100, 255), TRUE);
		SetFontSize(14);
		DrawTextUtf8(SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT - 78, GetColor(255, 180, 255), "チャージ");
	}

	if (spellBreak)
	{
		SetFontSize(22);
		DrawTextUtf8(SCREEN_WIDTH / 2 - 140, 130, GetColor(255, 255, 200), "スペルカード破壊！");
	}

#if SHOW_FPS
	SetFontSize(16);
	DrawFormatString(SCREEN_WIDTH - 280, 8, GetColor(180, 180, 180),
		"FPS:%d  敵弾:%d/%d", GetFPS(), m_Bullets.GetActiveEnemyBulletCount(), MAX_ACTIVE_ENEMY_BULLETS);
#endif

	// 被弾ゲームオーバー時の赤フェードアウト
	if (!m_Player.IsAlive() && m_ClearDelayTimer > 0)
	{
		int fadeAlpha = (int)(255.0f * ((float)m_ClearDelayTimer / 90.0f));
		if (fadeAlpha > 255) fadeAlpha = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
		DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(40, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		SetFontSize(56);
		DrawTextUtf8(SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 30, GetColor(255, 50, 50), "ミッション失敗…");
	}

	// クリア時のフェードアウト
	if (m_Enemies.IsAllWavesComplete() && m_ClearDelayTimer > 0)
	{
		int fadeAlpha = (int)(255.0f * ((float)m_ClearDelayTimer / 120.0f));
		if (fadeAlpha > 255) fadeAlpha = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
		DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(20, 30, 60), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		SetFontSize(56);
		DrawTextUtf8(SCREEN_WIDTH / 2 - 220, SCREEN_HEIGHT / 2 - 30, GetColor(0, 220, 255), "ミッション完遂！");
	}

	SetFontSize(20);
}