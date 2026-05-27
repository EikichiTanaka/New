#include "Common/ResourceManager.h"
#include "GameConfig.h"
#include "DxLib.h"
#include <cmath>

// ============================================================
// ResourceManager
//   - ?e: ?R?[?h????????t???X?v???C?g?i?m???????j
//   - ???@/?G/?{?X: DxLib ?v???~?e?B?u?iCone/Cube/Triangle/Sphere?j??
//                  ?g??????? "3D ???f??" ?\???Baxis-aligned ?z?u??y??B
//   - ?C??? assets/models/*.mv1 ??????? MV1 ??D??i???[?U?[????????p?j
// ============================================================

namespace
{
	// --- ?X?v???C?g ---
	int s_BulletPlayer = -1;
	int s_BulletEnemy = -1;

	// --- ?C?? MV1 ???f???i???????v???~?e?B?u?????j---
	int s_PlayerModel = -1;
	int s_EnemyModel  = -1;
	int s_BossPhaseModels[BOSS_PHASE_MODEL_COUNT] = { -1, -1, -1, -1 };

	bool s_Ready = false;

	int CreateGraphOnScreen(int w, int h, void (*drawFunc)(int w, int h))
	{
		int prev = GetDrawScreen();
		int screen = MakeScreen(w, h, TRUE); // ???t??
		if (screen == -1) return -1;

		SetDrawScreen(screen);
		// ???S?????N???A
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 0);
		DrawBox(0, 0, w, h, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		drawFunc(w, h);

		SetDrawScreen(prev);
		return screen;
	}

	// ???S?????\?t?g?~?i???i?K?j??O???[?e?????
	void ProcBulletPlayer(int w, int h)
	{
		const int cx = w / 2;
		const int cy = h / 2;
		const int maxR = w / 2 - 1;

		// ?O???n???[?i?????j
		for (int r = maxR; r >= maxR / 2; r--)
		{
			int alpha = 30 - (maxR - r) * 2;
			if (alpha < 4) alpha = 4;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(cx, cy, r, GetColor(0, 200, 255), TRUE);
		}
		// ?{??
		for (int r = maxR / 2; r >= 2; r--)
		{
			int alpha = 70 + (maxR / 2 - r) * 12;
			if (alpha > 255) alpha = 255;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			int rg = 200 + (maxR / 2 - r) * 5;
			if (rg > 255) rg = 255;
			DrawCircle(cx, cy, r, GetColor(rg, 255, 255), TRUE);
		}
		// ???S?n?C???C?g
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		DrawCircle(cx, cy, 2, GetColor(255, 255, 255), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	void ProcBulletEnemy(int w, int h)
	{
		const int cx = w / 2;
		const int cy = h / 2;
		const int maxR = w / 2 - 1;

		for (int r = maxR; r >= maxR / 2; r--)
		{
			int alpha = 28 - (maxR - r) * 2;
			if (alpha < 4) alpha = 4;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			DrawCircle(cx, cy, r, GetColor(255, 60, 120), TRUE);
		}
		for (int r = maxR / 2; r >= 2; r--)
		{
			int alpha = 80 + (maxR / 2 - r) * 12;
			if (alpha > 255) alpha = 255;
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
			int b = 120 + (maxR / 2 - r) * 8;
			if (b > 255) b = 255;
			DrawCircle(cx, cy, r, GetColor(255, 80, b), TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 255);
		DrawCircle(cx, cy, 2, GetColor(255, 240, 240), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// --- 3D ?v???~?e?B?u????: ???@ ---
	// ???@?? +Z ?????i?G????j???????????O??
	void DrawPlayerMesh(float x, float y, float z, bool fever)
	{
		const float s = PLAYER_DRAW_SIZE;
		unsigned int bodyCol = fever ? GetColor(255, 215, 60) : GetColor(0, 220, 255);
		unsigned int wingCol = fever ? GetColor(255, 180, 40) : GetColor(0, 160, 220);
		unsigned int coreCol = fever ? GetColor(255, 255, 200) : GetColor(220, 255, 255);
		unsigned int engCol  = fever ? GetColor(255, 120, 40) : GetColor(255, 80, 200);

		// ?@??R?[???i?O???? +Z?j
		VECTOR noseTip  = VGet(x, y, z + s * 1.6f);
		VECTOR noseBase = VGet(x, y, z + s * 0.2f);
		DrawCone3D(noseTip, noseBase, s * 0.45f, 6, bodyCol, coreCol, FALSE);

		// ????i?y????????1??j
		VECTOR bodyMin = VGet(x - s * 0.45f, y - s * 0.30f, z - s * 0.6f);
		VECTOR bodyMax = VGet(x + s * 0.45f, y + s * 0.30f, z + s * 0.4f);
		DrawCube3D(bodyMin, bodyMax, bodyCol, coreCol, FALSE);

		// ???i?O?p?`2???j
		VECTOR wL1 = VGet(x - s * 0.45f, y, z + s * 0.10f);
		VECTOR wL2 = VGet(x - s * 1.40f, y, z - s * 0.45f);
		VECTOR wL3 = VGet(x - s * 0.45f, y, z - s * 0.55f);
		DrawTriangle3D(wL1, wL2, wL3, wingCol, TRUE);

		VECTOR wR1 = VGet(x + s * 0.45f, y, z + s * 0.10f);
		VECTOR wR2 = VGet(x + s * 1.40f, y, z - s * 0.45f);
		VECTOR wR3 = VGet(x + s * 0.45f, y, z - s * 0.55f);
		DrawTriangle3D(wR1, wR2, wR3, wingCol, TRUE);

		// ?G???W??????i????j
		VECTOR engBase = VGet(x, y, z - s * 0.6f);
		VECTOR engTip  = VGet(x, y, z - s * 1.15f - sinf((float)GetNowCount() / 60.0f) * 1.5f);
		DrawCone3D(engBase, engTip, s * 0.28f, 5, engCol, engCol, FALSE);

		// ?R?N?s?b?g
		DrawSphere3D(VGet(x, y + s * 0.15f, z + s * 0.1f), s * 0.22f, 5, coreCol, coreCol, FALSE);

		// ?t?B?[?o?[???I?[??
		if (fever && VISUAL_RICH)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 110);
			DrawSphere3D(VGet(x, y, z), s * 1.4f, 6, engCol, engCol, FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// --- 3D ?v???~?e?B?u????: ?G?i?^?C?v??`??????j---
	// ?G?? -Z ?????i???@????j???????????O??
	void DrawEnemyMesh(EnemyType type, float x, float y, float z, float r, int seed)
	{
		const float s = r;
		float bob = sinf((float)seed / 22.0f) * 1.5f;
		y += bob;

		unsigned int bodyCol, accentCol, coreCol;
		switch (type)
		{
		case EnemyType::Spinner:
			bodyCol = GetColor(180, 100, 30);
			accentCol = GetColor(255, 200, 60);
			coreCol = GetColor(255, 255, 200);
			break;
		case EnemyType::Tank:
			bodyCol = GetColor(120, 30, 140);
			accentCol = GetColor(220, 80, 255);
			coreCol = GetColor(255, 220, 255);
			break;
		case EnemyType::Shield:
			bodyCol = GetColor(30, 80, 160);
			accentCol = GetColor(80, 160, 255);
			coreCol = GetColor(200, 230, 255);
			break;
		case EnemyType::Splitter:
			bodyCol = GetColor(180, 80, 30);
			accentCol = GetColor(255, 160, 60);
			coreCol = GetColor(255, 230, 180);
			break;
		case EnemyType::Eraser:
			bodyCol = GetColor(30, 140, 90);
			accentCol = GetColor(60, 255, 160);
			coreCol = GetColor(200, 255, 230);
			break;
		case EnemyType::Sniper:
			bodyCol = GetColor(160, 160, 30);
			accentCol = GetColor(255, 255, 100);
			coreCol = GetColor(255, 255, 200);
			break;
		case EnemyType::Basic:
		default:
			bodyCol = GetColor(160, 30, 30);
			accentCol = GetColor(255, 80, 80);
			coreCol = GetColor(255, 200, 200);
			break;
		}

		// ?@??R?[???i-Z ?????j
		VECTOR noseTip  = VGet(x, y, z - s * 1.0f);
		VECTOR noseBase = VGet(x, y, z + s * 0.1f);
		DrawCone3D(noseTip, noseBase, s * 0.45f, 6, accentCol, coreCol, FALSE);

		// ????
		float bw = s * 0.55f;
		float bh = s * 0.40f;
		float bd = s * 0.65f;
		if (type == EnemyType::Tank || type == EnemyType::Shield)
		{
			bw = s * 0.75f;
			bh = s * 0.55f;
		}
		VECTOR bodyMin = VGet(x - bw, y - bh, z - bd);
		VECTOR bodyMax = VGet(x + bw, y + bh, z + bd);
		DrawCube3D(bodyMin, bodyMax, bodyCol, accentCol, FALSE);

		// ???^?t?B??
		if (type == EnemyType::Spinner || type == EnemyType::Eraser)
		{
			// ??]?????i4???j
			float ang = (float)seed * 0.08f;
			for (int k = 0; k < 4; k++)
			{
				float a = ang + (float)k * 1.5708f;
				VECTOR p1 = VGet(x, y + s * 0.1f, z);
				VECTOR p2 = VGet(x + cosf(a) * s * 1.2f, y + s * 0.1f, z + sinf(a) * s * 1.2f);
				VECTOR p3 = VGet(x + cosf(a + 0.3f) * s * 0.6f, y + s * 0.1f, z + sinf(a + 0.3f) * s * 0.6f);
				DrawTriangle3D(p1, p2, p3, accentCol, TRUE);
			}
		}
		else
		{
			// ???E??O?p?t?B??
			VECTOR wL1 = VGet(x - bw, y, z + s * 0.20f);
			VECTOR wL2 = VGet(x - bw - s * 0.9f, y, z - s * 0.20f);
			VECTOR wL3 = VGet(x - bw, y, z - s * 0.55f);
			DrawTriangle3D(wL1, wL2, wL3, accentCol, TRUE);

			VECTOR wR1 = VGet(x + bw, y, z + s * 0.20f);
			VECTOR wR2 = VGet(x + bw + s * 0.9f, y, z - s * 0.20f);
			VECTOR wR3 = VGet(x + bw, y, z - s * 0.55f);
			DrawTriangle3D(wR1, wR2, wR3, accentCol, TRUE);
		}

		// ?R?A
		DrawSphere3D(VGet(x, y, z - s * 0.4f), s * 0.20f, 5, coreCol, coreCol, FALSE);

		// Shield ?^?C?v??O?????????o???A
		if (type == EnemyType::Shield)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
			DrawSphere3D(VGet(x, y, z - s * 0.8f), s * 0.8f, 6, accentCol, accentCol, FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// --- 3D ?v???~?e?B?u????: ?{?X?i???d?????O?{?{??j---
	void DrawBossMesh(float x, float y, float z, float r, int phase, int timer)
	{
		unsigned int coreCol, ringCol, accCol;
		switch (phase)
		{
		case 0: coreCol = GetColor(80, 10, 60);  ringCol = GetColor(255, 60, 160); accCol = GetColor(255, 220, 255); break;
		case 1: coreCol = GetColor(80, 50, 0);   ringCol = GetColor(255, 180, 40); accCol = GetColor(255, 240, 180); break;
		case 2: coreCol = GetColor(60, 0, 60);   ringCol = GetColor(220, 60, 255); accCol = GetColor(255, 200, 255); break;
		default:coreCol = GetColor(80, 0, 0);    ringCol = GetColor(255, 50, 80);  accCol = GetColor(255, 200, 200); break;
		}

		VECTOR c = VGet(x, y, z);

		// ?{???
		DrawSphere3D(c, r * 0.8f, 10, coreCol, ringCol, FALSE);

		// ????????O?i??????????6??j
		float spin = (float)timer * 0.04f;
		const int ringCount = 6;
		for (int i = 0; i < ringCount; i++)
		{
			float a = spin + (float)i * (6.2831853f / ringCount);
			float rr = r * 1.3f + sinf((float)timer / 20.0f + i) * 4.0f;
			VECTOR p = VGet(x + cosf(a) * rr, y + sinf((float)timer * 0.05f + i) * 6.0f, z + sinf(a) * rr);
			DrawSphere3D(p, r * 0.18f, 6, ringCol, accCol, FALSE);
		}

		// ?????p?i?R?[??2?{?j
		VECTOR hornBaseL = VGet(x - r * 0.4f, y + r * 0.4f, z);
		VECTOR hornTipL  = VGet(x - r * 0.6f, y + r * 1.3f, z);
		DrawCone3D(hornTipL, hornBaseL, r * 0.16f, 5, ringCol, accCol, FALSE);

		VECTOR hornBaseR = VGet(x + r * 0.4f, y + r * 0.4f, z);
		VECTOR hornTipR  = VGet(x + r * 0.6f, y + r * 1.3f, z);
		DrawCone3D(hornTipR, hornBaseR, r * 0.16f, 5, ringCol, accCol, FALSE);

		// ???S?R?A
		DrawSphere3D(c, r * 0.3f + sinf((float)timer / 8.0f) * 2.0f, 6, accCol, accCol, FALSE);

		// ?I?[??
		if (VISUAL_RICH)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, 110);
			DrawSphere3D(c, r * 1.2f, 8, ringCol, ringCol, FALSE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		}
	}

	// --- 3D ?v???~?e?B?u????: ???{?X ---
	void DrawMidBossMesh(float x, float y, float z, float r, int midId, int timer)
	{
		unsigned int bodyCol = (midId == 1) ? GetColor(140, 30, 90) : GetColor(140, 90, 30);
		unsigned int accCol  = (midId == 1) ? GetColor(255, 100, 180) : GetColor(255, 180, 60);
		unsigned int coreCol = GetColor(255, 220, 255);

		VECTOR c = VGet(x, y, z);
		DrawSphere3D(c, r * 0.75f, 8, bodyCol, accCol, FALSE);

		// 4??????t?B??
		float spin = (float)timer * 0.05f;
		for (int k = 0; k < 4; k++)
		{
			float a = spin + (float)k * 1.5708f;
			VECTOR p1 = VGet(x, y, z);
			VECTOR p2 = VGet(x + cosf(a) * r * 1.4f, y, z + sinf(a) * r * 1.4f);
			VECTOR p3 = VGet(x + cosf(a + 0.4f) * r * 0.7f, y, z + sinf(a + 0.4f) * r * 0.7f);
			DrawTriangle3D(p1, p2, p3, accCol, TRUE);
		}

		DrawSphere3D(c, r * 0.3f, 6, coreCol, coreCol, FALSE);
	}

	void DrawBulletBillboard(float x, float y, float z, float size, float angleDeg, int graph)
	{
		if (graph == -1) return;
		VECTOR pos = VGet(x, y, z);
		DrawBillboard3D(pos, 0.5f, 0.5f, size, angleDeg, graph, TRUE);
	}

	constexpr float DegToRad = 3.14159265f / 180.0f;

	int TryLoadModelPath(const char* path)
	{
		if (path == nullptr || path[0] == '\0') return -1;
		return MV1LoadModel(path);
	}

	int TryLoadModelPaths(const char* primary, const char* fallback)
	{
		int m = TryLoadModelPath(primary);
		if (m >= 0) return m;
		return TryLoadModelPath(fallback);
	}

	float EnemyTypeScaleMul(EnemyType type)
	{
		switch (type)
		{
		case EnemyType::Tank:
		case EnemyType::Shield:
			return 1.35f;
		case EnemyType::Splitter:
		case EnemyType::Eraser:
			return 1.15f;
		case EnemyType::Sniper:
			return 1.05f;
		case EnemyType::Spinner:
			return 0.95f;
		default:
			return 1.0f;
		}
	}

	void DrawMV1Model(int handle, float x, float y, float z, float gameRadius,
		const ModelDrawSettings& settings, float extraMul = 1.0f)
	{
		if (handle < 0) return;

		float sx = settings.scaleX * extraMul;
		float sy = settings.scaleY * extraMul;
		float sz = settings.scaleZ * extraMul;

		if (settings.radiusRef > 0.01f && gameRadius > 0.01f)
		{
			const float mul = (gameRadius / settings.radiusRef) * settings.radiusScaleMul;
			sx *= mul;
			sy *= mul;
			sz *= mul;
		}

		MV1SetScale(handle, VGet(sx, sy, sz));
		MV1SetRotationXYZ(handle, VGet(
			settings.rotXDeg * DegToRad,
			settings.rotYDeg * DegToRad,
			settings.rotZDeg * DegToRad));
		MV1SetPosition(handle, VGet(
			x + settings.offsetX,
			y + settings.offsetY,
			z + settings.offsetZ));
		MV1DrawModel(handle);
	}

	int ResolveBossPhaseModelHandle(int phase)
	{
		if (phase < 0) phase = 0;
		if (phase >= BOSS_PHASE_MODEL_COUNT) phase = BOSS_PHASE_MODEL_COUNT - 1;

		if (s_BossPhaseModels[phase] >= 0)
			return s_BossPhaseModels[phase];

		if (s_BossPhaseModels[0] >= 0)
			return s_BossPhaseModels[0];

		for (int i = 0; i < BOSS_PHASE_MODEL_COUNT; i++)
		{
			if (s_BossPhaseModels[i] >= 0)
				return s_BossPhaseModels[i];
		}
		return -1;
	}
}

namespace ResourceManager
{

void Init()
{
	Final();

	// ?e??R?[?h??????m???????
	s_BulletPlayer = CreateGraphOnScreen(48, 48, ProcBulletPlayer);
	s_BulletEnemy  = CreateGraphOnScreen(48, 48, ProcBulletEnemy);

	// ?C??? MV1 ???f???i??????? -1 ????A?v???~?e?B?u?`???t?H?[???o?b?N?j
	s_PlayerModel = TryLoadModelPaths(PLAYER_MODEL_PATH_PRIMARY, PLAYER_MODEL_PATH_FALLBACK);
	s_EnemyModel  = TryLoadModelPaths(ENEMY_MODEL_PATH_PRIMARY, ENEMY_MODEL_PATH_FALLBACK);

	for (int i = 0; i < BOSS_PHASE_MODEL_COUNT; i++)
		s_BossPhaseModels[i] = TryLoadModelPath(BOSS_PHASE_MODEL_PATHS[i]);

	s_Ready = (s_BulletPlayer != -1 && s_BulletEnemy != -1);
}

void Final()
{
	auto delG = [](int& g) {
		if (g != -1) { DeleteGraph(g); g = -1; }
	};
	auto delM = [](int& m) {
		if (m != -1) { MV1DeleteModel(m); m = -1; }
	};
	delG(s_BulletPlayer);
	delG(s_BulletEnemy);
	delM(s_PlayerModel);
	delM(s_EnemyModel);
	for (int i = 0; i < BOSS_PHASE_MODEL_COUNT; i++)
		delM(s_BossPhaseModels[i]);
	s_Ready = false;
}

bool IsReady()
{
	return s_Ready;
}

bool HasPlayerModel()
{
	return s_PlayerModel >= 0;
}

bool HasEnemyModel()
{
	return s_EnemyModel >= 0;
}

bool HasBossModel()
{
	for (int i = 0; i < BOSS_PHASE_MODEL_COUNT; i++)
	{
		if (s_BossPhaseModels[i] >= 0)
			return true;
	}
	return false;
}

bool HasBossPhaseModel(int phase)
{
	if (phase < 0 || phase >= BOSS_PHASE_MODEL_COUNT)
		return false;
	return s_BossPhaseModels[phase] >= 0;
}

void DrawPlayer(float x, float y, float z, bool fever)
{
	if (s_PlayerModel >= 0)
	{
		const float feverMul = fever ? 1.08f : 1.0f;
		DrawMV1Model(s_PlayerModel, x, y, z, PLAYER_DRAW_SIZE, PLAYER_MV1_SETTINGS, feverMul);
		return;
	}
	DrawPlayerMesh(x, y, z, fever);
}

void DrawPlayerBullet(float x, float y, float z, float vx, float vz, float collisionRadius)
{
	float angle = atan2f(vx, vz) * 57.2957795f;
	float size = PlayerBulletVisualRadiusFromCollision(collisionRadius);
	if (size < 10.0f) size = 10.0f;
	DrawBulletBillboard(x, y, z, size, angle, s_BulletPlayer);
}

void DrawEnemyBullet(float x, float y, float z, float collisionRadius)
{
	float size = EnemyBulletVisualRadiusFromCollision(collisionRadius);
	if (size < 9.0f) size = 9.0f;
	DrawBulletBillboard(x, y, z, size, 0.0f, s_BulletEnemy);
}

void DrawEnemy(EnemyType type, float x, float y, float z, float radius, int animSeed)
{
	(void)animSeed;
	if (s_EnemyModel >= 0)
	{
		DrawMV1Model(s_EnemyModel, x, y, z, radius, ENEMY_MV1_SETTINGS, EnemyTypeScaleMul(type));
		return;
	}
	DrawEnemyMesh(type, x, y, z, radius, animSeed);
}

void DrawBoss(float x, float y, float z, float radius, int phase, int timer)
{
	(void)timer;
	const int handle = ResolveBossPhaseModelHandle(phase);
	if (handle >= 0)
	{
		DrawMV1Model(handle, x, y, z, radius, GetBossPhaseModelSettings(phase), 1.0f);
		return;
	}
	DrawBossMesh(x, y, z, radius, phase, timer);
}

void DrawMidBoss(float x, float y, float z, float radius, int midId, int timer)
{
	(void)midId;
	(void)timer;
	const int handle = ResolveBossPhaseModelHandle(0);
	if (handle >= 0)
	{
		DrawMV1Model(handle, x, y, z, radius, MIDBOSS_MV1_SETTINGS, 1.0f);
		return;
	}
	DrawMidBossMesh(x, y, z, radius, midId, timer);
}

} // namespace ResourceManager
