#include "Game/CollisionDebug.h"
#include "Game/Player.h"
#include "Game/BulletManager.h"
#include "Game/EnemyManager.h"
#include "Game/Enemy.h"
#include "Common/GameScreen.h"
#include "Common/UiDraw.h"
#include "GameConfig.h"
#include "DxLib.h"

namespace
{
	bool s_Visible = false;

	void DrawWireSphere(float x, float y, float z, float radius, unsigned int color, int alpha = 150)
	{
		if (radius < 0.5f) return;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawSphere3D(VGet(x, y, z), radius, 10, color, color, FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

namespace CollisionDebug
{

bool IsVisible()
{
	return s_Visible;
}

void SetVisible(bool on)
{
	s_Visible = on;
}

void Toggle()
{
	s_Visible = !s_Visible;
}

void Draw(const Player& player, const BulletManager& bullets, const EnemyManager& enemies)
{
	if (!s_Visible)
		return;

	const float py = PLAYER_Y;
	const float px = player.GetX();
	const float pz = player.GetZ();

	// Player: body / graze
	DrawWireSphere(px, py, pz, PLAYER_COLLISION_RADIUS, GetColor(255, 255, 255), 200);
	DrawWireSphere(px, py, pz, PLAYER_GRAZE_RADIUS, GetColor(80, 255, 120), 90);

	// Player bullets
	const int activePB = bullets.GetActivePlayerBulletCount();
	const Bullet* pSlots = bullets.GetPlayerBullets();
	for (int li = 0; li < activePB; li++)
	{
		const int i = bullets.GetActivePlayerBulletSlot(li);
		const Bullet& b = pSlots[i];
		DrawWireSphere(b.x, py, b.z, b.radius, GetColor(100, 255, 255), 130);
	}

	// Enemy bullets
	const int activeEB = bullets.GetActiveEnemyBulletCount();
	const Bullet* eSlots = bullets.GetEnemyBullets();
	for (int li = 0; li < activeEB; li++)
	{
		const int i = bullets.GetActiveEnemyBulletSlot(li);
		const Bullet& b = eSlots[i];
		if (!b.active) continue;
		DrawWireSphere(b.x, py, b.z, b.radius, GetColor(255, 80, 120), 120);
	}

	// Grunt enemies
	const Enemy* pEnemies = enemies.GetEnemies();
	const int maxE = enemies.GetEnemyMax();
	for (int j = 0; j < maxE; j++)
	{
		if (!pEnemies[j].IsActive()) continue;
		DrawWireSphere(pEnemies[j].GetX(), py, pEnemies[j].GetZ(),
			pEnemies[j].GetRadius(), GetColor(255, 160, 60), 140);
	}

	// Mid-boss: hit / draw ref
	if (enemies.IsMidBossActive())
	{
		DrawWireSphere(enemies.GetMidBossX(), py, enemies.GetMidBossZ(),
			enemies.GetMidBossRadius(), GetColor(255, 255, 80), 170);
		DrawWireSphere(enemies.GetMidBossX(), py, enemies.GetMidBossZ(),
			enemies.GetMidBossDrawRadius(), GetColor(80, 140, 255), 70);
	}

	// Boss: hit / draw ref
	if (enemies.IsBossActive())
	{
		DrawWireSphere(enemies.GetBossX(), py, enemies.GetBossZ(),
			enemies.GetBossRadius(), GetColor(255, 220, 60), 170);
		DrawWireSphere(enemies.GetBossX(), py, enemies.GetBossZ(),
			enemies.GetBossDrawRadius(), GetColor(100, 160, 255), 70);
	}
}

void DrawHudIndicator()
{
	if (!s_Visible)
		return;

	GameScreenSyncSize();
	BeginScreenSpaceDraw();
	SetFontSize(16);
	DrawTextUtf8(12, SCREEN_HEIGHT - 52, GetColor(255, 255, 120),
		u8"\u5f53\u305f\u308a\u5224\u5b9a\u8868\u793a ON  (F3\u3067\u5207\u66ff)");
	SetFontSize(14);
	DrawTextUtf8(12, SCREEN_HEIGHT - 32, GetColor(180, 200, 220),
		u8"\u767d=\u81ea\u6a5f \u7dd1=\u304b\u3059\u308a \u8d64=\u6575\u5f48 \u6c34=\u81ea\u5f48 "
		u8"\u6a59=\u6575 \u9ec4=\u30dc\u30b9\u5f53\u305f\u308a \u9752=\u63cf\u753b\u57fa\u6e96");
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

} // namespace CollisionDebug
