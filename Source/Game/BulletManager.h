#pragma once
#include "Game/Bullet.h"
#include "Game/BulletPool.h"
#include "GameConfig.h"

class BulletManager
{
public:
	void Init();
	void Update(float playerX, float playerZ);
	void DrawLit() const;
	void DrawEnemiesUnlit() const;

	bool CanAddEnemyBullet() const { return m_EnemyPool.GetActiveCount() < MAX_ACTIVE_ENEMY_BULLETS; }

	void AddPlayerBullet(float x, float y, float z, float vx, float vy, float vz,
		float radius, unsigned int color, PlayerBulletKind kind = PlayerBulletKind::Normal,
		int pierce = 0, int hitDamage = 1);

	bool AddEnemyBullet(float x, float y, float z, float vx, float vy, float vz, float radius, unsigned int color);

	void ClearEnemyBullets();

	void ReleasePlayerBullet(int slotIndex);
	void ReleaseEnemyBullet(int slotIndex);

	const Bullet* GetPlayerBullets() const { return m_PlayerPool.GetSlots(); }
	Bullet* GetPlayerBulletsMutable() { return m_PlayerPool.GetSlots(); }
	int GetPlayerBulletMax() const { return PBULLET_MAX; }

	Bullet* GetEnemyBullets() { return m_EnemyPool.GetSlots(); }
	const Bullet* GetEnemyBullets() const { return m_EnemyPool.GetSlots(); }
	int GetEnemyBulletMax() const { return EBULLET_MAX; }
	int GetActiveEnemyBulletCount() const { return m_EnemyPool.GetActiveCount(); }

	int GetActivePlayerBulletCount() const { return m_PlayerPool.GetActiveCount(); }
	int GetActivePlayerBulletSlot(int listIndex) const { return m_PlayerPool.GetActiveIndex(listIndex); }
	int GetActiveEnemyBulletSlot(int listIndex) const { return m_EnemyPool.GetActiveIndex(listIndex); }

private:
	void SetupPlayerBullet(Bullet& b, float x, float y, float z,
		float vx, float vy, float vz, float radius, unsigned int color,
		PlayerBulletKind kind, int pierce, int hitDamage);

	void SetupEnemyBullet(Bullet& b, float x, float y, float z,
		float vx, float vy, float vz, float radius, unsigned int color);

	void DrawPlayerBullets3D() const;
	void DrawEnemyBullets3DUnlit() const;

	BulletPool<PBULLET_MAX> m_PlayerPool;
	BulletPool<EBULLET_MAX> m_EnemyPool;

	float m_RefPlayerX;
	float m_RefPlayerZ;
};
