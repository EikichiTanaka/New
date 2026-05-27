#include "Game/BulletManager.h"
#include "Game/TouhouTheme.h"
#include "Common/ResourceManager.h"
#include "Common/UiDraw.h"
#include "GameConfig.h"
#include "Common/GameOptions.h"
#include "DxLib.h"
#include <cmath>

void BulletManager::Init()
{
	m_PlayerPool.Init();
	m_EnemyPool.Init();
	m_RefPlayerX = 0.0f;
	m_RefPlayerZ = 0.0f;
}

void BulletManager::SetupPlayerBullet(Bullet& b, float x, float y, float z,
	float vx, float vy, float vz, float radius, unsigned int color,
	PlayerBulletKind kind, int pierce, int hitDamage)
{
	b.x = x;
	b.y = y;
	b.z = z;
	b.vx = vx;
	b.vy = vy;
	b.vz = vz;
	b.radius = PlayerBulletCollisionRadius(radius);
	b.color = color;
	b.life = 0;
	b.kind = kind;
	b.pierceLeft = pierce;
	b.hitDamage = hitDamage;
	b.active = true;
}

void BulletManager::SetupEnemyBullet(Bullet& b, float x, float y, float z,
	float vx, float vy, float vz, float radius, unsigned int color)
{
	b.x = x;
	b.y = y;
	b.z = z;
	b.vx = vx;
	b.vy = vy;
	b.vz = vz;
	b.radius = EnemyBulletCollisionRadius(radius);
	b.color = color;
	b.life = 0;
	b.kind = PlayerBulletKind::Normal;
	b.pierceLeft = 0;
	b.hitDamage = 1;
	b.active = true;
}

void BulletManager::AddPlayerBullet(float x, float y, float z, float vx, float vy, float vz,
	float radius, unsigned int color, PlayerBulletKind kind, int pierce, int hitDamage)
{
	Bullet* b = m_PlayerPool.Acquire();
	if (b == nullptr)
		return;
	SetupPlayerBullet(*b, x, y, z, vx, vy, vz, radius, color, kind, pierce, hitDamage);
}

bool BulletManager::AddEnemyBullet(float x, float y, float z, float vx, float vy, float vz, float radius, unsigned int color)
{
	if (!CanAddEnemyBullet())
		return false;

	Bullet* b = m_EnemyPool.Acquire();
	if (b == nullptr)
		return false;

	SetupEnemyBullet(*b, x, y, z, vx, vy, vz, radius, color);
	return true;
}

void BulletManager::ClearEnemyBullets()
{
	m_EnemyPool.ReleaseAll();
}

void BulletManager::ReleasePlayerBullet(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= PBULLET_MAX)
		return;
	if (!m_PlayerPool.GetSlots()[slotIndex].active)
		return;
	m_PlayerPool.Release(slotIndex);
}

void BulletManager::ReleaseEnemyBullet(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= EBULLET_MAX)
		return;
	if (!m_EnemyPool.GetSlots()[slotIndex].active)
		return;
	m_EnemyPool.Release(slotIndex);
}

void BulletManager::Update(float playerX, float playerZ)
{
	m_RefPlayerX = playerX;
	m_RefPlayerZ = playerZ;

	const float limitX = FIELD_HALF_W + 100.0f;
	const float limitZ = FIELD_HALF_D + 100.0f;

	for (int li = m_PlayerPool.GetActiveCount() - 1; li >= 0; li--)
	{
		const int idx = m_PlayerPool.GetActiveIndex(li);
		Bullet& b = m_PlayerPool.GetSlots()[idx];

		if (b.kind == PlayerBulletKind::Homing)
		{
			b.vz += 0.22f;
			b.vx += -b.x * 0.012f;
			float spd = sqrtf(b.vx * b.vx + b.vz * b.vz);
			const float maxSpd = PBULLET_SPEED * 1.15f;
			if (spd > maxSpd && spd > 0.01f)
			{
				b.vx = b.vx / spd * maxSpd;
				b.vz = b.vz / spd * maxSpd;
			}
		}

		b.x += b.vx;
		b.y += b.vy;
		b.z += b.vz;

		if (b.life > 0)
			b.life--;

		if (b.x < -limitX || b.x > limitX || b.z < -limitZ || b.z > limitZ)
			m_PlayerPool.Release(idx);
	}

	for (int li = m_EnemyPool.GetActiveCount() - 1; li >= 0; li--)
	{
		const int idx = m_EnemyPool.GetActiveIndex(li);
		Bullet& b = m_EnemyPool.GetSlots()[idx];

		b.x += b.vx;
		b.y += b.vy;
		b.z += b.vz;

		if (b.x < -limitX || b.x > limitX || b.z < -limitZ || b.z > limitZ)
			m_EnemyPool.Release(idx);
	}
}

void BulletManager::DrawPlayerBullets3D() const
{
	const int count = m_PlayerPool.GetActiveCount();

	if (PreferGameSprites() && ResourceManager::IsReady())
	{
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_PlayerPool.GetSlots()[m_PlayerPool.GetActiveIndex(li)];
			ResourceManager::DrawPlayerBullet(b.x, b.y, b.z, b.vx, b.vz, b.radius);
		}
		return;
	}

	const int seg = BULLET_DRAW_SEG_PLAYER;
	const unsigned int edge = GetColor(220, 255, 255);

	if (UseTouhouTheme())
	{
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_PlayerPool.GetSlots()[m_PlayerPool.GetActiveIndex(li)];
			TouhouTheme::DrawOfudaBullet(b.x, b.y, b.z, b.vx, b.vz, b.radius, b.color);
		}
		return;
	}

	if (VISUAL_RICH)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, RICH_PLAYER_HALO_ALPHA);
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_PlayerPool.GetSlots()[m_PlayerPool.GetActiveIndex(li)];
			const float vr = PlayerBulletVisualRadiusFromCollision(b.radius);
			DrawSphere3D(VGet(b.x, b.y, b.z), vr * 1.4f, seg, b.color, b.color, FALSE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_PlayerPool.GetSlots()[m_PlayerPool.GetActiveIndex(li)];
			const float vr = PlayerBulletVisualRadiusFromCollision(b.radius);
			DrawSphere3D(VGet(b.x, b.y, b.z), vr, seg, b.color, edge, FALSE);
		}
		return;
	}

	for (int li = 0; li < count; li++)
	{
		const Bullet& b = m_PlayerPool.GetSlots()[m_PlayerPool.GetActiveIndex(li)];
		const float vr = PlayerBulletVisualRadiusFromCollision(b.radius);
		DrawSphere3D(VGet(b.x, b.y, b.z), vr, seg, b.color, edge, FALSE);
	}
}

void BulletManager::DrawLit() const
{
	BeginBulletDraw3D();
	DrawPlayerBullets3D();
}

void BulletManager::DrawEnemyBullets3DUnlit() const
{
	const int seg = BULLET_DRAW_SEG_ENEMY;
	const int alpha = (int)(g_Options.bulletAlpha * 255.0f);
	const bool useAlpha = (alpha < 254);

	const float cullX = BULLET_DRAW_CULL_DIST_X;
	const float cullZ = BULLET_DRAW_CULL_DIST_Z;
	const int count = m_EnemyPool.GetActiveCount();

	if (PreferGameSprites() && ResourceManager::IsReady())
	{
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_EnemyPool.GetSlots()[m_EnemyPool.GetActiveIndex(li)];
			const float dx = b.x - m_RefPlayerX;
			const float dz = b.z - m_RefPlayerZ;
			if (fabsf(dx) > cullX || fabsf(dz) > cullZ) continue;
			ResourceManager::DrawEnemyBullet(b.x, b.y, b.z, b.radius);
		}
		return;
	}

	if (UseTouhouTheme())
	{
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_EnemyPool.GetSlots()[m_EnemyPool.GetActiveIndex(li)];
			const float dx = b.x - m_RefPlayerX;
			const float dz = b.z - m_RefPlayerZ;
			if (fabsf(dx) > cullX || fabsf(dz) > cullZ) continue;
			TouhouTheme::DrawStarBullet(b.x, b.y, b.z, b.radius, b.color);
		}
		return;
	}

	if (VISUAL_RICH)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, RICH_BULLET_GLOW_ALPHA);
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_EnemyPool.GetSlots()[m_EnemyPool.GetActiveIndex(li)];
			const float dx = b.x - m_RefPlayerX;
			const float dz = b.z - m_RefPlayerZ;
			if (fabsf(dx) > cullX || fabsf(dz) > cullZ) continue;
			const float vr = EnemyBulletVisualRadiusFromCollision(b.radius);
			DrawSphere3D(VGet(b.x, b.y, b.z), vr * 1.8f, seg, b.color, b.color, FALSE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		const unsigned int core = GetColor(255, 255, 255);
		for (int li = 0; li < count; li++)
		{
			const Bullet& b = m_EnemyPool.GetSlots()[m_EnemyPool.GetActiveIndex(li)];
			const float dx = b.x - m_RefPlayerX;
			const float dz = b.z - m_RefPlayerZ;
			if (fabsf(dx) > cullX || fabsf(dz) > cullZ) continue;
			const float vr = EnemyBulletVisualRadiusFromCollision(b.radius);
			DrawSphere3D(VGet(b.x, b.y, b.z), vr, seg, b.color, core, FALSE);
		}
		return;
	}

	if (useAlpha)
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

	for (int li = 0; li < count; li++)
	{
		const Bullet& b = m_EnemyPool.GetSlots()[m_EnemyPool.GetActiveIndex(li)];
		const float dx = b.x - m_RefPlayerX;
		const float dz = b.z - m_RefPlayerZ;
		if (fabsf(dx) > cullX || fabsf(dz) > cullZ)
			continue;

		const float vr = EnemyBulletVisualRadiusFromCollision(b.radius);
		DrawSphere3D(VGet(b.x, b.y, b.z), vr, seg, b.color, b.color, FALSE);
	}

	if (useAlpha)
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void BulletManager::DrawEnemiesUnlit() const
{
	BeginBulletDraw3D();
	DrawEnemyBullets3DUnlit();
}
