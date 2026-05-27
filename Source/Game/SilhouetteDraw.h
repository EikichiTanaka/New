#pragma once

#include "GameConfig.h"
#include "Game/Enemy.h"

// [VISUAL_STYLE] ???E???????????ADrawCube3D / DrawCapsule3D / DrawLine3D ??
// ???J?E?l?^????V???G?b?g??`???i?????????]??????a????j
namespace SilhouetteDraw
{
	void DrawPlayerMecha(float x, float y, float z,
		unsigned int bodyColor, unsigned int accentColor, bool fever);

	void DrawEnemy(EnemyType type, float x, float y, float z, float radius,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);

	void DrawMidBoss(float x, float y, float z, float radius, int bossId,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);

	void DrawBoss(float x, float y, float z, float radius, int phase,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);

	// [VISUAL_THEME] �������E�d���V���G�b�g
	void DrawEnemyYoukai(EnemyType type, float x, float y, float z, float radius,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);

	void DrawMidBossYoukai(float x, float y, float z, float radius, int bossId,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);

	void DrawBossYoukai(float x, float y, float z, float radius, int phase,
		unsigned int bodyColor, unsigned int accentColor, int animSeed);
}
