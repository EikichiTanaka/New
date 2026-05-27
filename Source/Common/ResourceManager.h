#pragma once

#include "Game/Enemy.h"
#include "DxLib.h"

// ?Q?[???p?X?v???C?g???????E?`??i?N????1??[?h?A?r???{?[?h?`??j
namespace ResourceManager
{
	void Init();
	void Final();
	bool IsReady();

	void DrawPlayer(float x, float y, float z, bool fever);
	void DrawPlayerBullet(float x, float y, float z, float vx, float vz, float radius);
	void DrawEnemyBullet(float x, float y, float z, float radius);
	void DrawEnemy(EnemyType type, float x, float y, float z, float radius, int animSeed);
	void DrawBoss(float x, float y, float z, float radius, int phase, int timer);
	void DrawMidBoss(float x, float y, float z, float radius, int midId, int timer);

	bool HasPlayerModel();
	bool HasEnemyModel();
	bool HasBossModel();
	bool HasBossPhaseModel(int phase);
}
