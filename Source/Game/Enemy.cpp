#include "Game/Enemy.h"
#include "DxLib.h"
#include <cmath>

// --- Init: ?G??\??l???????x?E??????u????? ---
void Enemy::Init(EnemyType type, float startX, float startZ, Difficulty diff)
{
	m_X = startX;
	m_Y = PLAYER_Y; // ?e??????@??????????????@?????Y???W?????
	m_Z = startZ;
	m_Type = type;
	m_Radius = ENEMY_DRAW_SIZE;
	m_Active = true;
	
	// ???????????t???[?????????o?????????
	m_Timer = GetRand(30);
	m_HitCooldown = 0;

	// ???x???e??????
	switch (diff)
	{
	case Difficulty::Easy:
		m_BulletSpeed = EBULLET_SPEED_EASY;
		break;
	case Difficulty::Hard:
		m_BulletSpeed = EBULLET_SPEED_HARD;
		break;
	case Difficulty::Lunatic:
		m_BulletSpeed = EBULLET_SPEED_LUNATIC;
		break;
	case Difficulty::Normal:
	default:
		m_BulletSpeed = EBULLET_SPEED_NORMAL;
		break;
	}

	// ?^?C?v???\??l????
	switch (m_Type)
	{
	case EnemyType::Spinner:
		m_Hp = 6;
		m_MaxHp = 6;
		m_VX = 2.5f;
		m_VZ = -0.6f;
		m_FireRate = 32;
		break;

	case EnemyType::Tank:
		m_Hp = 16;
		m_MaxHp = 16;
		m_VX = 0.0f;
		m_VZ = -0.3f;
		m_FireRate = 26;
		break;

	case EnemyType::Shield:
		m_Hp = 12;
		m_MaxHp = 12;
		m_VX = 0.0f;
		m_VZ = -0.9f;
		m_FireRate = 36;
		break;

	case EnemyType::Splitter:
		m_Hp = 5;
		m_MaxHp = 5;
		m_VX = 1.2f;
		m_VZ = -1.0f;
		m_FireRate = 40;
		break;

	case EnemyType::Eraser:
		m_Hp = 4;
		m_MaxHp = 4;
		m_VX = 0.0f;
		m_VZ = -0.5f;
		m_FireRate = 22;
		break;

	case EnemyType::Sniper:
		m_Hp = 2;
		m_MaxHp = 2;
		m_VX = 0.0f;
		m_VZ = -0.4f;
		m_FireRate = 18;
		break;

	case EnemyType::Basic:
	default:
		m_Hp = 3;
		m_MaxHp = 3;
		m_VX = 0.0f;
		m_VZ = -1.6f; // ????????????i
		m_FireRate = 30;
		break;
	}
}

// --- Update: ???????????p?^?[????X?V ---
void Enemy::Update(float playerX, float playerZ, BulletManager& bullets)
{
	if (!m_Active) return;

	if (m_HitCooldown > 0)
		m_HitCooldown--;

	m_Timer++;

	// ?^?C?v??????????
	switch (m_Type)
	{
	case EnemyType::Spinner:
		// ???E??T?C???E?F?[?u?????s????
		m_X += sinf((float)m_Timer / 15.0f) * m_VX * 1.5f;
		m_Z += m_VZ;
		break;

	case EnemyType::Eraser:
		m_X += sinf((float)m_Timer / 22.0f) * 2.0f;
		m_Z += m_VZ;
		break;

	case EnemyType::Splitter:
		m_X += sinf((float)m_Timer / 18.0f) * m_VX;
		m_Z += m_VZ;
		break;

	case EnemyType::Tank:
	case EnemyType::Shield:
	case EnemyType::Sniper:
	case EnemyType::Basic:
	default:
		m_X += m_VX;
		m_Z += m_VZ;
		break;
	}

	// ?t?B?[???h??O?i????@????z??????[???????????????
	if (m_Z < -FIELD_HALF_D - 50.0f)
	{
		m_Active = false;
		return;
	}

	// ?t?B?[???h???E??N?????v?i??????h?~???
	if (m_X < -FIELD_HALF_W + 20.0f) m_X = -FIELD_HALF_W + 20.0f;
	if (m_X > FIELD_HALF_W - 20.0f)  m_X = FIELD_HALF_W - 20.0f;

	// ????????
	if (m_Timer % m_FireRate == 0)
	{
		float dx = playerX - m_X;
		float dz = playerZ - m_Z;
		float dist = sqrtf(dx * dx + dz * dz);

		if (dist > 1.0f)
		{
			// ?x?N?g??????K??
			float ndx = dx / dist;
			float ndz = dz / dist;

			switch (m_Type)
			{
			case EnemyType::Basic:
				{
					float baseAngle = atan2f(ndz, ndx);
					float spreads[3] = { -0.14f, 0.0f, 0.14f };
					for (int i = 0; i < 3; i++)
					{
						float angle = baseAngle + spreads[i];
						bullets.AddEnemyBullet(m_X, m_Y, m_Z,
							cosf(angle) * m_BulletSpeed, 0.0f, sinf(angle) * m_BulletSpeed,
							EBULLET_RADIUS * 0.95f, GetColor(255, 50, 100));
					}
				}
				break;

			case EnemyType::Spinner:
				{
					int bulletCount = 18;
					float spin = (float)m_Timer * 0.05f;
					for (int i = 0; i < bulletCount; i++)
					{
						float angle = (i * 2.0f * 3.14159265f) / bulletCount + spin;
						bullets.AddEnemyBullet(m_X, m_Y, m_Z,
							cosf(angle) * m_BulletSpeed * 0.8f, 0.0f, sinf(angle) * m_BulletSpeed * 0.8f,
							EBULLET_RADIUS * 0.9f, GetColor(255, 100, 255));
					}
					bullets.AddEnemyBullet(m_X, m_Y, m_Z,
						ndx * m_BulletSpeed * 1.1f, 0.0f, ndz * m_BulletSpeed * 1.1f,
						EBULLET_RADIUS * 1.1f, GetColor(255, 200, 255));
				}
				break;

			case EnemyType::Tank:
				{
					float baseAngle = atan2f(ndz, ndx);
					float offsets[5] = { -0.28f, -0.14f, 0.0f, 0.14f, 0.28f };
					for (int i = 0; i < 5; i++)
					{
						float angle = baseAngle + offsets[i];
						bullets.AddEnemyBullet(m_X, m_Y, m_Z,
							cosf(angle) * m_BulletSpeed * 0.95f, 0.0f, sinf(angle) * m_BulletSpeed * 0.95f,
							EBULLET_RADIUS * 1.15f, GetColor(255, 0, 50));
					}
					for (int i = 0; i < 8; i++)
					{
						float angle = (i * 2.0f * 3.14159265f) / 8.0f;
						bullets.AddEnemyBullet(m_X, m_Y, m_Z,
							cosf(angle) * m_BulletSpeed * 0.65f, 0.0f, sinf(angle) * m_BulletSpeed * 0.65f,
							EBULLET_RADIUS, GetColor(200, 0, 255));
					}
				}
				break;

			case EnemyType::Sniper:
				bullets.AddEnemyBullet(m_X, m_Y, m_Z,
					ndx * m_BulletSpeed * 1.35f, 0.0f, ndz * m_BulletSpeed * 1.35f,
					EBULLET_RADIUS * 0.85f, GetColor(255, 255, 80));
				break;

			case EnemyType::Eraser:
				for (int i = 0; i < 12; i++)
				{
					float angle = (i * 2.0f * 3.14159265f) / 12.0f + m_Timer * 0.04f;
					bullets.AddEnemyBullet(m_X, m_Y, m_Z,
						cosf(angle) * m_BulletSpeed * 0.75f, 0.0f, sinf(angle) * m_BulletSpeed * 0.75f,
						EBULLET_RADIUS * 1.05f, GetColor(80, 255, 200));
				}
				break;

			case EnemyType::Shield:
				bullets.AddEnemyBullet(m_X, m_Y, m_Z,
					ndx * m_BulletSpeed, 0.0f, ndz * m_BulletSpeed,
					EBULLET_RADIUS, GetColor(100, 200, 255));
				break;

			case EnemyType::Splitter:
				bullets.AddEnemyBullet(m_X, m_Y, m_Z,
					ndx * m_BulletSpeed * 0.9f, 0.0f, ndz * m_BulletSpeed * 0.9f,
					EBULLET_RADIUS, GetColor(255, 150, 50));
				break;
			}
		}
	}
}

// --- Draw: ?G???????HP?o????`?? ---
void Enemy::Draw() const
{
	if (!m_Active) return;

	// ?G??O???L???[??
	VECTOR minPos = VGet(m_X - m_Radius, m_Y - m_Radius, m_Z - m_Radius);
	VECTOR maxPos = VGet(m_X + m_Radius, m_Y + m_Radius, m_Z + m_Radius);

	unsigned int bodyColor, edgeColor;

	switch (m_Type)
	{
	case EnemyType::Spinner:
		bodyColor = GetColor(255, 160, 40);
		edgeColor = GetColor(255, 255, 150);
		break;
	case EnemyType::Tank:
		bodyColor = GetColor(220, 80, 255);
		edgeColor = GetColor(255, 200, 255);
		break;
	case EnemyType::Shield:
		bodyColor = GetColor(80, 140, 255);
		edgeColor = GetColor(180, 220, 255);
		break;
	case EnemyType::Splitter:
		bodyColor = GetColor(255, 140, 60);
		edgeColor = GetColor(255, 220, 120);
		break;
	case EnemyType::Eraser:
		bodyColor = GetColor(60, 255, 160);
		edgeColor = GetColor(180, 255, 220);
		break;
	case EnemyType::Sniper:
		bodyColor = GetColor(255, 255, 100);
		edgeColor = GetColor(255, 255, 200);
		break;
	case EnemyType::Basic:
	default:
		bodyColor = GetColor(255, 70, 70);   // ??
		edgeColor = GetColor(255, 200, 200);
		break;
	}

	// ?G??3D??????????`??
	DrawCube3D(minPos, maxPos, bodyColor, edgeColor, TRUE);

	if ((GetNowCount() % ENEMY_HP_BAR_DRAW_INTERVAL) != 0)
		return;

	VECTOR hpWorldPos = VGet(m_X, m_Y + m_Radius + 6.0f, m_Z);
	VECTOR hpScreenPos = ConvWorldPosToScreenPos(hpWorldPos);

	if (hpScreenPos.z >= 0.0f && hpScreenPos.z <= 1.0f)
	{
		int sx = (int)hpScreenPos.x;
		int sy = (int)hpScreenPos.y;
		int barHalfWidth = 20;
		int barHeight = 4;

		// ?Q?[?W?w?i????O???[???
		DrawBox(sx - barHalfWidth, sy - barHeight, sx + barHalfWidth, sy, GetColor(60, 60, 60), TRUE);

		// ?c??HP????
		float hpRatio = (float)m_Hp / m_MaxHp;
		int greenBarWidth = (int)((barHalfWidth * 2) * hpRatio);

		// ?Q?[?W?{??i???C???O???[?????
		DrawBox(sx - barHalfWidth, sy - barHeight, sx - barHalfWidth + greenBarWidth, sy, GetColor(0, 255, 128), TRUE);
	}
}

bool Enemy::TryApplyBulletDamage(int dmg)
{
	if (!m_Active || m_HitCooldown > 0)
		return false;

	m_HitCooldown = ENEMY_HIT_IFRAMES;
	TakeDamage(dmg);
	return true;
}

void Enemy::TakeDamage(int dmg)
{
	if (!m_Active) return;

	m_Hp -= dmg;
	if (m_Hp <= 0)
	{
		m_Hp = 0;
		m_Active = false;
	}
}
