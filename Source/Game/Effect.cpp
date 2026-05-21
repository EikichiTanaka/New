#include "Game/Effect.h"
#include "GameConfig.h"
#include "Common/GameScreen.h"
#include "DxLib.h"
#include <cmath>

void Effect::Init()
{
	m_BombActive = false;
	m_BombRadius = 0.0f;
	m_BombTimer = 0;
	m_BombX = m_BombY = m_BombZ = 0.0f;

	for (int i = 0; i < PARTICLE_MAX; i++)
		m_Particles[i].active = false;

	for (int i = 0; i < STAR_MAX; i++)
	{
		m_Stars[i].x = (float)(GetRand(1000) - 500);
		m_Stars[i].y = (float)(GetRand(250) - 100);
		m_Stars[i].z = (float)(GetRand(1200) - 600);
		m_Stars[i].speed = 12.0f + (float)GetRand(80) / 10.0f;
		int brightness = 80 + GetRand(175);
		m_Stars[i].color = GetColor(brightness, brightness, brightness);
	}
}

void Effect::Update()
{
	for (int i = 0; i < STAR_MAX; i++)
	{
		m_Stars[i].z -= m_Stars[i].speed;
		if (m_Stars[i].z < -650.0f)
		{
			m_Stars[i].x = (float)(GetRand(1000) - 500);
			m_Stars[i].y = (float)(GetRand(250) - 100);
			m_Stars[i].z = 600.0f;
			m_Stars[i].speed = 12.0f + (float)GetRand(80) / 10.0f;
		}
	}

	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		if (!m_Particles[i].active)
			continue;

		m_Particles[i].x += m_Particles[i].vx;
		m_Particles[i].y += m_Particles[i].vy;
		m_Particles[i].z += m_Particles[i].vz;
		m_Particles[i].vx *= 0.94f;
		m_Particles[i].vy *= 0.94f;
		m_Particles[i].vz *= 0.94f;
		m_Particles[i].life--;
		if (m_Particles[i].life <= 0)
			m_Particles[i].active = false;
	}

	if (m_BombActive)
	{
		m_BombTimer++;
		m_BombRadius += 18.0f;
		if (m_BombTimer >= 48)
		{
			m_BombActive = false;
			m_BombRadius = 0.0f;
		}
	}
}

void Effect::Draw() const
{
	const int cx = SCREEN_WIDTH / 2;
	const int cy = SCREEN_HEIGHT / 4;

	SetDrawBlendMode(DX_BLENDMODE_ADD, 200);
	const int starDrawMax = EFFECT_STAR_DRAW_MAX < STAR_MAX ? EFFECT_STAR_DRAW_MAX : STAR_MAX;
	for (int i = 0; i < starDrawMax; i++)
	{
		float depth = (m_Stars[i].z + 650.0f) / 1250.0f;
		if (depth < 0.04f || depth > 1.0f)
			continue;

		float scale = 0.35f + depth * 0.55f;
		int sx = cx + (int)(m_Stars[i].x * scale);
		int sy = cy + (int)(m_Stars[i].y * 0.35f);
		int r = 1 + (int)((1.0f - depth) * 2.0f);
		DrawCircle(sx, sy, r, m_Stars[i].color, TRUE);
	}

	if (m_BombActive)
	{
		int alpha = 200 - (m_BombTimer * 4);
		if (alpha < 0) alpha = 0;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		VECTOR center = VGet(m_BombX, m_BombY, m_BombZ);
		DrawSphere3D(center, m_BombRadius, 8, GetColor(0, 255, 255), GetColor(0, 255, 255), FALSE);
	}

	int drawn = 0;
	int lastAlpha = -1;
	for (int i = 0; i < PARTICLE_MAX && drawn < EFFECT_MAX_DRAW_PARTICLES; i++)
	{
		if (!m_Particles[i].active)
			continue;

		float lifeRatio = (float)m_Particles[i].life / (float)m_Particles[i].maxLife;
		int alpha = (int)(255.0f * lifeRatio);
		if (alpha != lastAlpha)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			lastAlpha = alpha;
		}

		VECTOR sp = ConvWorldPosToScreenPos(VGet(m_Particles[i].x, m_Particles[i].y, m_Particles[i].z));
		if (sp.z < 0.0f || sp.z > 1.0f)
			continue;
		int r = (int)(m_Particles[i].radius * (1.5f - sp.z * 0.35f));
		if (r < 2) r = 2;
		DrawCircle((int)sp.x, (int)sp.y, r, m_Particles[i].color, TRUE);
		drawn++;
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Effect::AddExplosion(float x, float y, float z, unsigned int color, int count)
{
	if (count <= 0)
		count = EXPLOSION_PARTICLE_COUNT;
	int spawned = 0;
	for (int i = 0; i < PARTICLE_MAX && spawned < count; i++)
	{
		if (!m_Particles[i].active)
		{
			m_Particles[i].x = x;
			m_Particles[i].y = y;
			m_Particles[i].z = z;

			float phi = ((float)GetRand(360) * 3.14159f) / 180.0f;
			float theta = ((float)GetRand(180) * 3.14159f) / 180.0f;
			float speed = 3.5f + (float)GetRand(60) / 10.0f;

			m_Particles[i].vx = sinf(theta) * cosf(phi) * speed;
			m_Particles[i].vy = sinf(theta) * sinf(phi) * speed * 0.2f;
			m_Particles[i].vz = cosf(theta) * speed;

			m_Particles[i].radius = 2.5f + (float)GetRand(25) / 10.0f;
			m_Particles[i].maxLife = 25 + GetRand(20);
			m_Particles[i].life = m_Particles[i].maxLife;
			m_Particles[i].color = color;
			m_Particles[i].active = true;
			spawned++;
		}
	}
}

void Effect::AddGrazeSpark(float x, float y, float z)
{
	int spawned = 0;
	int count = GRAZE_SPARK_COUNT;
	for (int i = 0; i < PARTICLE_MAX && spawned < count; i++)
	{
		if (!m_Particles[i].active)
		{
			m_Particles[i].x = x;
			m_Particles[i].y = y;
			m_Particles[i].z = z;
			m_Particles[i].vx = (float)(GetRand(80) - 40) / 10.0f;
			m_Particles[i].vy = (float)(GetRand(10) - 5) / 10.0f;
			m_Particles[i].vz = -2.0f - (float)GetRand(40) / 10.0f;
			m_Particles[i].radius = 1.5f + (float)GetRand(10) / 10.0f;
			m_Particles[i].maxLife = 10 + GetRand(10);
			m_Particles[i].life = m_Particles[i].maxLife;
			m_Particles[i].color = GetColor(0, 255, 255);
			m_Particles[i].active = true;
			spawned++;
		}
	}
}

void Effect::AddItemSparkle(float x, float y, float z, unsigned int color)
{
	int spawned = 0;
	for (int i = 0; i < PARTICLE_MAX && spawned < 3; i++)
	{
		if (!m_Particles[i].active)
		{
			m_Particles[i].x = x;
			m_Particles[i].y = y;
			m_Particles[i].z = z;
			m_Particles[i].vx = (float)(GetRand(60) - 30) / 10.0f;
			m_Particles[i].vy = (float)(GetRand(40) - 20) / 10.0f;
			m_Particles[i].vz = (float)(GetRand(60) - 30) / 10.0f;
			m_Particles[i].radius = 1.0f + (float)GetRand(8) / 10.0f;
			m_Particles[i].maxLife = 8 + GetRand(8);
			m_Particles[i].life = m_Particles[i].maxLife;
			m_Particles[i].color = color;
			m_Particles[i].active = true;
			spawned++;
		}
	}
}

void Effect::TriggerBombShockwave(float x, float y, float z)
{
	m_BombActive = true;
	m_BombTimer = 0;
	m_BombRadius = 0.0f;
	m_BombX = x;
	m_BombY = y;
	m_BombZ = z;
}
