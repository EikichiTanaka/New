#pragma once

enum class PlayerBulletKind : char
{
	Normal = 0,
	Pierce,
	Homing,
	Charge
};

struct Bullet
{
	float x, y, z;
	float vx, vy, vz;
	float radius;
	unsigned int color;
	int life; // 敵弾: グレイズ済み。自機弾: ボス/中ボス判定スキップ残フレーム
	bool active;
	PlayerBulletKind kind;
	int pierceLeft;
	int hitDamage;
};
