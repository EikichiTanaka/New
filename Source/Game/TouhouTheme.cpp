#include "Game/TouhouTheme.h"
#include "GameConfig.h"
#include "Common/GameScreen.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	int s_MikoGraph = -1;
	int s_OfudaGraph = -1;
	bool s_Ready = false;

	int CreateGraphOnScreen(int w, int h, bool useAlpha, void (*drawFunc)(int w, int h))
	{
		int prev = GetDrawScreen();
		int screen = MakeScreen(w, h, useAlpha ? TRUE : FALSE);
		if (screen == -1) return -1;

		SetDrawScreen(screen);
		if (useAlpha)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 0);
			DrawBox(0, 0, w, h, GetColor(0, 0, 0), TRUE);
		}
		else
		{
			DrawBox(0, 0, w, h, GetColor(255, 0, 255), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		drawFunc(w, h);
		SetDrawScreen(prev);
		return screen;
	}

	void DrawProceduralMiko(int w, int h)
	{
		const int cx = w / 2;
		// ??
		DrawCircle(cx, 22, 18, GetColor(25, 20, 35), TRUE);
		// ??
		DrawCircle(cx, 26, 14, GetColor(255, 220, 200), TRUE);
		// ???{??
		DrawBox(cx - 14, 34, cx + 14, 40, GetColor(220, 40, 60), TRUE);
		// ????
		DrawBox(cx - 22, 38, cx + 22, 72, GetColor(245, 245, 250), TRUE);
		// ?��i??j
		DrawTriangle(cx, 72, cx - 26, h - 8, cx + 26, h - 8, GetColor(190, 35, 55), TRUE);
		// ??
		DrawBox(cx - 30, 44, cx - 18, 68, GetColor(240, 240, 245), TRUE);
		DrawBox(cx + 18, 44, cx + 30, 68, GetColor(240, 240, 245), TRUE);
		// ??
		DrawBox(cx - 18, 64, cx + 18, 70, GetColor(255, 210, 80), TRUE);
	}

	void DrawProceduralOfuda(int w, int h)
	{
		DrawBox(2, 2, w - 2, h - 2, GetColor(245, 240, 220), TRUE);
		DrawBox(0, 0, w, h, GetColor(30, 20, 10), FALSE);
		for (int i = 0; i < 4; i++)
		{
			int ly = 10 + i * 8;
			DrawBox(6, ly, w - 6, ly + 3, GetColor(200, 40, 50), TRUE);
		}
		DrawCircle(w / 2, h - 10, 4, GetColor(200, 40, 50), TRUE);
	}
}

namespace TouhouTheme
{

void Init()
{
	Final();

	int loaded = LoadGraph(TOUHOU_MIKO_ASSET_PATH, TRUE);
	if (loaded != -1)
		s_MikoGraph = loaded;
	else
		s_MikoGraph = CreateGraphOnScreen(96, 128, true, DrawProceduralMiko);

	s_OfudaGraph = CreateGraphOnScreen(28, 44, true, DrawProceduralOfuda);
	s_Ready = (s_MikoGraph != -1 && s_OfudaGraph != -1);
}

void Final()
{
	if (s_MikoGraph != -1)
	{
		DeleteGraph(s_MikoGraph);
		s_MikoGraph = -1;
	}
	if (s_OfudaGraph != -1)
	{
		DeleteGraph(s_OfudaGraph);
		s_OfudaGraph = -1;
	}
	s_Ready = false;
}

bool IsReady()
{
	return s_Ready;
}

void DrawPlayerBillboard(float x, float y, float z, bool fever)
{
	if (!s_Ready)
		Init();
	if (!s_Ready) return;

	VECTOR pos = VGet(x, y + 8.0f, z);
	float size = PLAYER_DRAW_SIZE * 5.5f;
	if (fever) size *= 1.08f;

	float bob = sinf((float)GetNowCount() / 140.0f) * 2.0f;
	pos.y += bob;

	int trans = TRUE;
	DrawBillboard3D(pos, 0.5f, 0.5f, size, 0.0f, s_MikoGraph, trans);

	if (fever)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
		DrawBillboard3D(pos, 0.5f, 0.5f, size * 1.15f, 0.0f, s_MikoGraph, trans);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void DrawCloudParallax(int frameCount)
{
	// ???O???f?i?????????j
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2, GetColor(255, 180, 140), TRUE);
	DrawBox(0, SCREEN_HEIGHT / 4, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(120, 90, 160), TRUE);
	DrawBox(0, SCREEN_HEIGHT * 2 / 3, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(40, 30, 70), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	struct Layer { int speed; int alpha; int count; int baseR; int baseG; int baseB; };
	const Layer layers[] = {
		{ 1, 50, 10, 255, 255, 255 },
		{ 2, 70, 12, 240, 230, 255 },
		{ 4, 90, 14, 220, 210, 245 },
	};

	for (const Layer& L : layers)
	{
		int scroll = (frameCount * L.speed) % SCREEN_WIDTH;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, L.alpha);
		for (int i = 0; i < L.count; i++)
		{
			int seed = i * 173 + L.speed * 31;
			int cx = (seed * 97 + scroll) % (SCREEN_WIDTH + 200) - 100;
			int cy = (seed * 53) % (SCREEN_HEIGHT - 80) + 20;
			int rx = 60 + (seed % 80);
			int ry = 20 + (seed % 25);
			unsigned int col = GetColor(L.baseR, L.baseG, L.baseB);
			DrawCircle(cx, cy, rx, col, FALSE);
			DrawCircle(cx + rx / 3, cy, rx * 2 / 3, col, FALSE);
			DrawCircle(cx - rx / 3, cy, rx * 2 / 3, col, FALSE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void DrawStarBullet(float x, float y, float z, float collisionRadius, unsigned int color)
{
	const float radius = EnemyBulletVisualRadiusFromCollision(collisionRadius);
	const float r = radius * 1.35f;
	VECTOR c = VGet(x, y, z);

	SetDrawBlendMode(DX_BLENDMODE_ADD, 100);
	DrawSphere3D(c, radius * 1.5f, 4, color, color, FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	unsigned int core = GetColor(255, 255, 255);
	DrawLine3D(VGet(x - r, y, z), VGet(x + r, y, z), core);
	DrawLine3D(VGet(x, y, z - r), VGet(x, y, z + r), core);
	DrawLine3D(VGet(x - r * 0.7f, y, z - r * 0.7f), VGet(x + r * 0.7f, y, z + r * 0.7f), color);
	DrawLine3D(VGet(x - r * 0.7f, y, z + r * 0.7f), VGet(x + r * 0.7f, y, z - r * 0.7f), color);
	DrawSphere3D(c, radius * 0.45f, 4, core, core, FALSE);
}

void DrawOfudaBullet(float x, float y, float z, float vx, float vz,
	float collisionRadius, unsigned int color)
{
	if (!s_Ready)
	{
		DrawStarBullet(x, y, z, collisionRadius, color);
		return;
	}

	float angle = atan2f(vx, vz) * 57.2957795f;
	const float radius = PlayerBulletVisualRadiusFromCollision(collisionRadius);
	float size = radius * 1.35f;
	if (size < 14.0f) size = 14.0f;

	VECTOR pos = VGet(x, y, z);
	DrawBillboard3D(pos, 0.5f, 0.5f, size, angle, s_OfudaGraph, TRUE);

	SetDrawBlendMode(DX_BLENDMODE_ADD, 80);
	DrawBillboard3D(pos, 0.5f, 0.5f, size * 1.1f, angle, s_OfudaGraph, TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

} // namespace TouhouTheme
