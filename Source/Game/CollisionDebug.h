#pragma once

class Player;
class BulletManager;
class EnemyManager;

// �����蔻��̃��C���[�\���i�Q�[���� F3 �� ON/OFF�j
namespace CollisionDebug
{
	bool IsVisible();
	void SetVisible(bool on);
	void Toggle();

	void Draw(const Player& player, const BulletManager& bullets, const EnemyManager& enemies);
	void DrawHudIndicator();
}
