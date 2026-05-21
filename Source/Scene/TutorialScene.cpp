#include "Scene/TutorialScene.h"
#include "GameConfig.h"
#include "Common/GameSession.h"
#include "Common/KeyHelper.h"
#include "Common/SoundSynth.h"
#include "Game/Collision.h"
#include "Common/UiDraw.h"
#include "DxLib.h"
#include <cmath>

void TutorialScene::Init()
{
	m_Step = 0;
	m_StepTimer = 0;
	m_BulletSpawnTimer = 0;
	m_Player.Init(true);
	m_Bullets.Init();
	m_Effect.Init();

	m_CamX = PLAYER_START_X;
	m_CamY = PLAYER_Y + 92.0f;
	m_CamZ = PLAYER_START_Z - 235.0f;
	m_LookX = PLAYER_START_X;
	m_LookY = PLAYER_Y + 32.0f;
	m_LookZ = PLAYER_START_Z + 155.0f;

	SoundSynth::Init();
}

void TutorialScene::SetupCamera()
{
	float px = m_Player.GetX();
	float pz = m_Player.GetZ();
	m_CamX += (px - m_CamX) * 0.16f;
	m_CamZ += ((pz - 235.0f) - m_CamZ) * 0.16f;
	m_LookX = px;
	m_LookZ = pz + 155.0f;
	SetCameraNearFar(1.0f, 3000.0f);
	SetCameraPositionAndTargetAndUpVec(
		VGet(m_CamX, m_CamY, m_CamZ),
		VGet(m_LookX, m_LookY, m_LookZ),
		VGet(0, 1, 0));
}

void TutorialScene::SpawnTutorialBullets()
{
	if (m_Step < 3) return;

	m_BulletSpawnTimer++;
	if (m_BulletSpawnTimer % 12 != 0) return;
	if (!m_Bullets.CanAddEnemyBullet()) return;

	float px = m_Player.GetX();
	float pz = m_Player.GetZ();
	float ex = px + (float)(GetRand(200) - 100);
	float ez = pz + 180.0f + (float)(GetRand(80));

	float dx = px - ex;
	float dz = pz - ez;
	float len = sqrtf(dx * dx + dz * dz);
	if (len < 1.0f) return;
	float spd = 3.5f;

	m_Bullets.AddEnemyBullet(ex, PLAYER_Y, ez,
		dx / len * spd, 0.0f, dz / len * spd,
		EBULLET_RADIUS * 0.9f, GetColor(255, 80, 140));
}

SceneType TutorialScene::Update()
{
	if (KeyHelper::IsCancelTrigger())
	{
		SoundSynth::Final();
		return SceneType::DifficultySelect;
	}

	if (KeyHelper::IsConfirmTrigger())
	{
		m_Step++;
		m_StepTimer = 0;
		if (m_Step >= 5)
		{
			SoundSynth::Final();
			return SceneType::DifficultySelect;
		}
	}

	m_StepTimer++;
	m_Player.Update(m_Bullets);
	SpawnTutorialBullets();
	m_Bullets.Update(m_Player.GetX(), m_Player.GetZ());
	m_Effect.Update();

	float px = m_Player.GetX();
	float pz = m_Player.GetZ();
	Bullet* eb = m_Bullets.GetEnemyBullets();
	int maxE = m_Bullets.GetEnemyBulletMax();
	for (int i = 0; i < maxE; i++)
	{
		if (!eb[i].active) continue;
		float dz = fabsf(eb[i].z - pz);
		float dx = fabsf(eb[i].x - px);
		if (dz > GRAZE_RADIUS + 40.0f && dx > GRAZE_RADIUS + 40.0f) continue;

		if (Collision::SphereVsSphere(eb[i].x, PLAYER_Y, eb[i].z, eb[i].radius, px, PLAYER_Y, pz, GRAZE_RADIUS))
		{
			if (eb[i].life != -99)
			{
				eb[i].life = -99;
				m_Player.AddGraze();
				m_Effect.AddGrazeSpark(px, PLAYER_Y, pz);
				SoundSynth::PlayGraze();
			}
		}
		if (Collision::SphereVsSphere(eb[i].x, PLAYER_Y, eb[i].z, eb[i].radius, px, PLAYER_Y, pz, m_Player.GetRadius()))
		{
			m_Bullets.ReleaseEnemyBullet(i);
		}
	}

	return SceneType::None;
}

void TutorialScene::DrawBackground2D() const
{
	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / (float)SCREEN_HEIGHT;
		int r = (int)(10 * (1.0f - t));
		int g = (int)(14 * (1.0f - t));
		int b = (int)(36 + 28 * (1.0f - t));
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(r, g, b));
	}

	for (int i = 0; i < 48; i++)
	{
		int sx = (i * 97 + m_StepTimer * 2) % SCREEN_WIDTH;
		int sy = (i * 41) % SCREEN_HEIGHT;
		int br = 70 + (i * 13) % 120;
		DrawPixel(sx, sy, GetColor(br, br, br + 20));
	}
}

void TutorialScene::Draw()
{
	DrawBackground2D();

	SetBackgroundColor(24, 36, 58);
	SetUseLighting(FALSE);
	SetupCamera();

	float halfW = FIELD_HALF_W;
	float halfD = FIELD_HALF_D;
	DrawCube3D(
		VGet(-halfW, -2, -halfD), VGet(halfW, 0, halfD),
		GetColor(25, 45, 70), GetColor(40, 70, 100), TRUE);

	float gridStep = 80.0f;
	unsigned int gridCol = GetColor(35, 55, 85);
	for (float gx = -halfW; gx <= halfW; gx += gridStep)
		DrawLine3D(VGet(gx, 0, -halfD), VGet(gx, 0, halfD), gridCol);
	for (float gz = -halfD; gz <= halfD; gz += gridStep)
		DrawLine3D(VGet(-halfW, 0, gz), VGet(halfW, 0, gz), gridCol);

	VECTOR pos = VGet(m_Player.GetX(), PLAYER_Y, m_Player.GetZ());
	DrawSphere3D(pos, PLAYER_DRAW_SIZE, 10, GetColor(0, 220, 255), GetColor(255, 255, 255), FALSE);

	m_Bullets.DrawLit();
	m_Bullets.DrawEnemiesUnlit();
	m_Effect.Draw();
	DrawHud();
	DrawStepText();
}

void TutorialScene::DrawHud()
{
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
	DrawBox(0, 0, SCREEN_WIDTH, 72, GetColor(0, 8, 22), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetFontSize(24);
	DrawTextUtf8(20, 12, GetColor(0, 255, 220), "チュートリアルモード");
	SetFontSize(18);
	DrawTextUtf8(20, 42, GetColor(200, 200, 200), "ESC:メニューへ  ENTER:次の説明");
}

void TutorialScene::DrawStepText()
{
	const char* texts[] = {
		"【1/5】移動: 方向キー / WASD で移動してみましょう",
		"【2/5】射撃: Z キー または Space で弾を撃てます",
		"【3/5】低速: Shift を押すと精密移動（チャージの準備にも）",
		"【4/5】かすり: 敵弾に近づくとグレイズ（弾は当たりません）",
		"【5/5】完了！ ENTER でメニューに戻り本編へ"
	};
	if (m_Step < 0 || m_Step > 4) return;

	const int boxTop = SCREEN_HEIGHT - 130;
	const int boxBottom = SCREEN_HEIGHT - 24;

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 210);
	DrawBox(40, boxTop, SCREEN_WIDTH - 40, boxBottom, GetColor(0, 12, 28), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	SetFontSize(22);
	DrawFormatString(60, boxTop + 28, GetColor(255, 255, 200), "%s", texts[m_Step]);
}
