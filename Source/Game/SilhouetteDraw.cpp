#include "Game/SilhouetteDraw.h"
#include "DxLib.h"
#include <cmath>

namespace
{
	void DrawBoxCentered(float cx, float cy, float cz,
		float hx, float hy, float hz,
		unsigned int fill, unsigned int edge)
	{
		DrawCube3D(
			VGet(cx - hx, cy - hy, cz - hz),
			VGet(cx + hx, cy + hy, cz + hz),
			fill, edge, FALSE);
	}

	void DrawCapsuleY(float cx, float y0, float cz, float y1, float r,
		unsigned int fill, unsigned int edge)
	{
		DrawCapsule3D(VGet(cx, y0, cz), VGet(cx, y1, cz), r, 6, fill, edge, FALSE);
	}

	void DrawHumanoid(float x, float y, float z, float scale,
		unsigned int body, unsigned int accent, int animSeed)
	{
		float sway = sinf((float)animSeed / 22.0f) * scale * 0.08f;
		float headY = y + scale * 1.05f;
		float shoulderY = y + scale * 0.55f;

		DrawSphere3D(VGet(x + sway, headY, z), scale * 0.32f, 6, body, accent, FALSE);
		DrawCapsuleY(x, y, z, shoulderY + scale * 0.15f, scale * 0.28f, body, accent);

		float armLen = scale * 0.75f;
		DrawLine3D(VGet(x, shoulderY, z), VGet(x - armLen, shoulderY - scale * 0.1f, z), accent);
		DrawLine3D(VGet(x, shoulderY, z), VGet(x + armLen, shoulderY - scale * 0.1f, z), accent);
		DrawLine3D(VGet(x, y, z), VGet(x - scale * 0.25f, y - scale * 0.55f, z), body);
		DrawLine3D(VGet(x, y, z), VGet(x + scale * 0.25f, y - scale * 0.55f, z), body);
	}
}

namespace SilhouetteDraw
{

void DrawPlayerMecha(float x, float y, float z,
	unsigned int bodyColor, unsigned int accentColor, bool fever)
{
	const float s = PLAYER_DRAW_SIZE;
	const unsigned int dark = GetColor(20, 40, 55);
	const unsigned int metal = fever ? GetColor(80, 70, 20) : GetColor(35, 55, 75);

	// ?n??V???h?E?i?G???j
	DrawBoxCentered(x, y - 3.0f, z, s * 1.1f, 1.5f, s * 0.75f, dark, dark);

	// ??
	DrawBoxCentered(x - s * 0.95f, y + s * 0.35f, z, s * 0.55f, s * 0.12f, s * 0.35f, metal, accentColor);
	DrawBoxCentered(x + s * 0.95f, y + s * 0.35f, z, s * 0.55f, s * 0.12f, s * 0.35f, metal, accentColor);

	// ????
	DrawCapsuleY(x, y, z, y + s * 1.05f, s * 0.32f, bodyColor, accentColor);

	// ?R?b?N?s?b?g
	DrawSphere3D(VGet(x, y + s * 0.95f, z + s * 0.2f), s * 0.28f, 6, accentColor, GetColor(255, 255, 255), FALSE);

	// ?m?[?Y?i?O?? +Z?j
	DrawBoxCentered(x, y + s * 0.45f, z + s * 0.65f, s * 0.18f, s * 0.18f, s * 0.45f, bodyColor, accentColor);

	// ?G???W???????i??? -Z?j
	DrawBoxCentered(x - s * 0.35f, y + s * 0.25f, z - s * 0.55f, s * 0.14f, s * 0.14f, s * 0.22f, dark, accentColor);
	DrawBoxCentered(x + s * 0.35f, y + s * 0.25f, z - s * 0.55f, s * 0.14f, s * 0.14f, s * 0.22f, dark, accentColor);

	// ????????
	DrawBoxCentered(x, y + s * 0.7f, z - s * 0.35f, s * 0.08f, s * 0.35f, s * 0.2f, metal, accentColor);
}

void DrawEnemy(EnemyType type, float x, float y, float z, float radius,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;

	switch (type)
	{
	case EnemyType::Tank:
		// ???d???
		DrawBoxCentered(x, y, z, r * 1.15f, r * 0.45f, r * 0.95f, bodyColor, accentColor);
		DrawBoxCentered(x, y + r * 0.55f, z + r * 0.15f, r * 0.35f, r * 0.35f, r * 0.7f, accentColor, GetColor(255, 255, 255));
		DrawLine3D(VGet(x, y + r * 0.55f, z + r * 0.85f), VGet(x, y + r * 0.55f, z + r * 1.35f), accentColor);
		break;

	case EnemyType::Spinner:
	{
		// ??]????~??iUFO?j
		float spin = (float)animSeed * 0.12f;
		DrawBoxCentered(x, y + r * 0.15f, z, r * 1.05f, r * 0.18f, r * 1.05f, bodyColor, accentColor);
		for (int i = 0; i < 6; i++)
		{
			float a = spin + (float)i * 3.14159265f / 3.0f;
			float ex = x + cosf(a) * r * 0.9f;
			float ez = z + sinf(a) * r * 0.9f;
			DrawLine3D(VGet(x, y + r * 0.2f, z), VGet(ex, y + r * 0.35f, ez), accentColor);
		}
		DrawSphere3D(VGet(x, y + r * 0.55f, z), r * 0.28f, 6, accentColor, GetColor(255, 255, 255), FALSE);
		break;
	}

	case EnemyType::Shield:
		// ?????\???????m
		DrawHumanoid(x, y, z - r * 0.15f, r * 0.85f, bodyColor, accentColor, animSeed);
		DrawBoxCentered(x, y + r * 0.45f, z + r * 0.75f, r * 0.12f, r * 0.75f, r * 0.55f, accentColor, GetColor(255, 255, 255));
		break;

	case EnemyType::Splitter:
		// ?o?q?R?A
		DrawBoxCentered(x - r * 0.45f, y + r * 0.35f, z, r * 0.42f, r * 0.55f, r * 0.42f, bodyColor, accentColor);
		DrawBoxCentered(x + r * 0.45f, y + r * 0.35f, z, r * 0.42f, r * 0.55f, r * 0.42f, bodyColor, accentColor);
		DrawLine3D(VGet(x - r * 0.45f, y + r * 0.35f, z), VGet(x + r * 0.45f, y + r * 0.35f, z), accentColor);
		break;

	case EnemyType::Eraser:
		// ???????H?`?@
		DrawBoxCentered(x, y + r * 0.1f, z, r * 1.25f, r * 0.22f, r * 0.65f, bodyColor, accentColor);
		DrawLine3D(VGet(x - r, y + r * 0.1f, z), VGet(x, y + r * 0.45f, z + r * 0.5f), accentColor);
		DrawLine3D(VGet(x + r, y + r * 0.1f, z), VGet(x, y + r * 0.45f, z + r * 0.5f), accentColor);
		DrawLine3D(VGet(x, y + r * 0.45f, z + r * 0.5f), VGet(x, y + r * 0.45f, z - r * 0.5f), accentColor);
		break;

	case EnemyType::Sniper:
		// ??????_????
		DrawBoxCentered(x, y + r * 0.55f, z, r * 0.28f, r * 0.95f, r * 0.28f, bodyColor, accentColor);
		DrawSphere3D(VGet(x, y + r * 1.15f, z), r * 0.22f, 6, accentColor, GetColor(255, 255, 255), FALSE);
		DrawLine3D(VGet(x, y + r * 1.05f, z), VGet(x, y + r * 1.05f, z + r * 1.1f), accentColor);
		break;

	case EnemyType::Basic:
	default:
		DrawHumanoid(x, y, z, r, bodyColor, accentColor, animSeed);
		break;
	}
}

void DrawMidBoss(float x, float y, float z, float radius, int bossId,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;
	float pulse = sinf((float)animSeed / 16.0f) * 2.0f;

	if (bossId == 1)
	{
		// ???????b?F????????^???J
		DrawBoxCentered(x, y, z, r * 1.35f, r * 0.55f, r * 0.9f, bodyColor, accentColor);
		DrawBoxCentered(x - r * 1.1f, y + r * 0.35f, z, r * 0.75f, r * 0.2f, r * 0.45f, accentColor, GetColor(255, 255, 255));
		DrawBoxCentered(x + r * 1.1f, y + r * 0.35f, z, r * 0.75f, r * 0.2f, r * 0.45f, accentColor, GetColor(255, 255, 255));
		DrawSphere3D(VGet(x, y + r * 0.75f + pulse, z), r * 0.4f, 8, accentColor, GetColor(255, 255, 255), FALSE);
	}
	else
	{
		// ?d?C??F?~??x?[?X?{?C??
		DrawBoxCentered(x, y, z, r * 1.2f, r * 0.35f, r * 1.2f, bodyColor, accentColor);
		DrawCapsuleY(x, y + r * 0.2f, z, y + r * 1.0f + pulse, r * 0.35f, accentColor, GetColor(255, 255, 255));
		for (int i = 0; i < 4; i++)
		{
			float a = (float)i * 1.5707963f;
			DrawLine3D(VGet(x, y + r * 0.15f, z),
				VGet(x + cosf(a) * r, y + r * 0.15f, z + sinf(a) * r), accentColor);
		}
	}
}

void DrawBoss(float x, float y, float z, float radius, int phase,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;
	float bob = sinf((float)animSeed / 12.0f) * 3.0f;

	// ???[?u??Z??????^?l?^?i?t?F?[?Y???????????????j
	DrawHumanoid(x, y + bob * 0.3f, z, r * 0.95f, bodyColor, accentColor, animSeed);

	// ?}???g?^?I?[???????O
	DrawBoxCentered(x, y + r * 0.35f, z - r * 0.35f, r * 1.4f, r * 0.08f, r * 0.95f, bodyColor, accentColor);

	// ???????^?R?A
	DrawSphere3D(VGet(x, y + r * 1.35f + bob, z), r * 0.35f, 8, accentColor, GetColor(255, 255, 255), FALSE);

	if (phase >= 1)
	{
		// ??2?`??F??????C??
		DrawBoxCentered(x - r * 0.95f, y + r * 0.75f, z + r * 0.35f, r * 0.35f, r * 0.25f, r * 0.55f, accentColor, GetColor(255, 255, 255));
		DrawBoxCentered(x + r * 0.95f, y + r * 0.75f, z + r * 0.35f, r * 0.35f, r * 0.25f, r * 0.55f, accentColor, GetColor(255, 255, 255));
	}
	if (phase >= 2)
	{
		// ??3?`??F?w?????
		for (int i = -2; i <= 2; i++)
		{
			DrawLine3D(VGet(x + (float)i * r * 0.25f, y + r * 0.9f, z - r * 0.5f),
				VGet(x + (float)i * r * 0.35f, y + r * 1.25f, z - r * 0.75f), accentColor);
		}
	}
}

// [VISUAL_THEME] ?d???V???G?b?g?i?^?C?v??j
void DrawEnemyYoukai(EnemyType type, float x, float y, float z, float radius,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;
	float wobble = sinf((float)animSeed / 20.0f + x * 0.02f) * r * 0.06f;

	switch (type)
	{
	case EnemyType::Spinner:
	{
		// ?V??F??`??H
		float spin = (float)animSeed * 0.1f;
		DrawCapsuleY(x, y + r * 0.2f, z, y + r * 0.9f, r * 0.22f, bodyColor, accentColor);
		for (int i = 0; i < 5; i++)
		{
			float a = spin + (float)i * 1.256637f;
			DrawLine3D(VGet(x, y + r * 0.55f, z),
				VGet(x + cosf(a) * r * 1.1f, y + r * 0.7f, z + sinf(a) * r * 1.1f), accentColor);
		}
		DrawSphere3D(VGet(x, y + r * 1.0f, z), r * 0.25f, 6, GetColor(80, 50, 40), accentColor, FALSE);
		break;
	}
	case EnemyType::Tank:
		// ?S?F?p?t?????
		DrawBoxCentered(x, y + r * 0.35f, z, r * 0.95f, r * 0.75f, r * 0.8f, bodyColor, accentColor);
		DrawLine3D(VGet(x - r * 0.5f, y + r * 1.0f, z - r * 0.3f), VGet(x - r * 0.7f, y + r * 1.45f, z - r * 0.2f), accentColor);
		DrawLine3D(VGet(x + r * 0.5f, y + r * 1.0f, z - r * 0.3f), VGet(x + r * 0.7f, y + r * 1.45f, z - r * 0.2f), accentColor);
		break;

	case EnemyType::Shield:
		// ????F?M?{?????
		DrawSphere3D(VGet(x, y + r * 0.45f + wobble, z), r * 0.55f, 6, bodyColor, accentColor, FALSE);
		DrawBoxCentered(x, y + r * 1.05f, z, r * 0.55f, r * 0.06f, r * 0.55f, accentColor, GetColor(255, 255, 255));
		break;

	case EnemyType::Splitter:
		// ?o?q?????
		DrawSphere3D(VGet(x - r * 0.5f, y + r * 0.5f, z), r * 0.42f, 6, bodyColor, accentColor, FALSE);
		DrawSphere3D(VGet(x + r * 0.5f, y + r * 0.5f, z), r * 0.42f, 6, bodyColor, accentColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 70);
		DrawSphere3D(VGet(x - r * 0.5f, y + r * 0.5f, z), r * 0.65f, 4, bodyColor, bodyColor, FALSE);
		DrawSphere3D(VGet(x + r * 0.5f, y + r * 0.5f, z), r * 0.65f, 4, bodyColor, bodyColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;

	case EnemyType::Eraser:
		// ?l???i??????j
		DrawCapsuleY(x, y, z, y + r * 1.1f, r * 0.35f, bodyColor, accentColor);
		SetDrawBlendMode(DX_BLENDMODE_ADD, 90);
		DrawSphere3D(VGet(x, y + r * 0.7f, z), r * 0.55f, 6, accentColor, accentColor, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		break;

	case EnemyType::Sniper:
		// ??????
		DrawCapsuleY(x, y, z, y + r * 0.5f, r * 0.12f, bodyColor, accentColor);
		DrawSphere3D(VGet(x, y + r * 0.85f, z), r * 0.5f, 8, GetColor(255, 255, 255), accentColor, FALSE);
		DrawSphere3D(VGet(x, y + r * 0.85f, z), r * 0.22f, 6, GetColor(20, 20, 40), GetColor(0, 0, 0), FALSE);
		break;

	case EnemyType::Basic:
	default:
		// ?????????m
		DrawLine3D(VGet(x, y, z), VGet(x, y + r * 0.35f, z), accentColor);
		DrawTriangle3D(
			VGet(x, y + r * 1.0f + wobble, z),
			VGet(x - r * 0.85f, y + r * 0.35f, z + r * 0.2f),
			VGet(x + r * 0.85f, y + r * 0.35f, z + r * 0.2f),
			bodyColor, TRUE);
		DrawSphere3D(VGet(x, y + r * 0.2f, z), r * 0.2f, 6, GetColor(240, 240, 240), accentColor, FALSE);
		break;
	}
}

void DrawMidBossYoukai(float x, float y, float z, float radius, int bossId,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;
	float pulse = sinf((float)animSeed / 14.0f) * 3.0f;

	if (bossId == 1)
	{
		// ??????
		DrawSphere3D(VGet(x, y + r * 0.55f, z), r * 0.55f, 8, bodyColor, accentColor, FALSE);
		for (int t = 0; t < 5; t++)
		{
			float a = (float)t * 1.2f + (float)animSeed * 0.04f;
			DrawLine3D(VGet(x, y + r * 0.5f, z - r * 0.3f),
				VGet(x + cosf(a) * r * 1.3f, y + r * 0.35f + pulse, z - r * 0.8f - sinf(a) * r * 0.4f),
				accentColor);
		}
	}
	else
	{
		// ??S
		DrawBoxCentered(x, y + r * 0.4f, z, r * 1.1f, r * 0.85f, r * 0.95f, bodyColor, accentColor);
		DrawLine3D(VGet(x - r * 0.6f, y + r * 1.1f, z), VGet(x - r * 0.85f, y + r * 1.6f + pulse, z), accentColor);
		DrawLine3D(VGet(x + r * 0.6f, y + r * 1.1f, z), VGet(x + r * 0.85f, y + r * 1.6f + pulse, z), accentColor);
		DrawSphere3D(VGet(x, y + r * 1.15f, z), r * 0.35f, 8, accentColor, GetColor(255, 255, 255), FALSE);
	}
}

void DrawBossYoukai(float x, float y, float z, float radius, int phase,
	unsigned int bodyColor, unsigned int accentColor, int animSeed)
{
	const float r = radius;
	float bob = sinf((float)animSeed / 10.0f) * 4.0f;

	// ??d????????V???G?b?g?{???d?I?[???????O
	DrawCapsuleY(x, y, z, y + r * 1.15f + bob, r * 0.42f, bodyColor, accentColor);
	DrawBoxCentered(x, y + r * 0.55f, z - r * 0.4f, r * 1.5f, r * 0.1f, r * 1.1f, bodyColor, accentColor);

	for (int ring = 0; ring < 3; ring++)
	{
		float rr = r * (1.0f + ring * 0.22f) + bob * 0.5f;
		int seg = 8 + ring * 4;
		for (int i = 0; i < seg; i++)
		{
			float a0 = (float)i / (float)seg * 6.283185f + (float)animSeed * 0.02f;
			float a1 = a0 + 6.283185f / (float)seg;
			DrawLine3D(
				VGet(x + cosf(a0) * rr, y + r * 0.65f, z + sinf(a0) * rr * 0.75f),
				VGet(x + cosf(a1) * rr, y + r * 0.65f, z + sinf(a1) * rr * 0.75f),
				accentColor);
		}
	}

	DrawSphere3D(VGet(x, y + r * 1.35f + bob, z), r * 0.38f, 8, accentColor, GetColor(255, 255, 255), FALSE);

	if (phase >= 1)
	{
		for (int i = -2; i <= 2; i++)
			DrawLine3D(VGet(x + (float)i * r * 0.3f, y + r * 0.9f, z + r * 0.5f),
				VGet(x + (float)i * r * 0.45f, y + r * 1.2f, z + r * 0.9f), accentColor);
	}
}

} // namespace SilhouetteDraw
