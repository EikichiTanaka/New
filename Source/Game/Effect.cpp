#include "Game/Effect.h"
#include "Common/GameScreen.h"
#include "DxLib.h"
#include <cmath>

void Effect::Init()
{
	m_BombActive = false;
	m_BombRadius = 0.0f;
	m_BombTimer = 0;
	m_BombX = m_BombY = m_BombZ = 0.0f;

	m_ParticlePool.Init();

	const int sw = SCREEN_WIDTH > 0 ? SCREEN_WIDTH : DESIGN_SCREEN_WIDTH;
	const int sh = SCREEN_HEIGHT > 0 ? SCREEN_HEIGHT : DESIGN_SCREEN_HEIGHT;
	for (int i = 0; i < STAR_MAX; i++)
	{
		m_Stars[i].screenX = (float)(GetRand(sw));
		m_Stars[i].screenY = (float)(GetRand(sh));
		m_Stars[i].speed = 1.2f + (float)GetRand(25) / 10.0f;
		int brightness = 80 + GetRand(175);
		m_Stars[i].color = GetColor(brightness, brightness, brightness + 20);
	}
}

void Effect::Update()
{
	const float h = (float)SCREEN_HEIGHT;
	const float w = (float)SCREEN_WIDTH;
	for (int i = 0; i < STAR_MAX; i++)
	{
		m_Stars[i].screenY += m_Stars[i].speed;
		if (m_Stars[i].screenY > h)
		{
			m_Stars[i].screenY = 0.0f;
			m_Stars[i].screenX = (float)(GetRand((int)w));
			m_Stars[i].speed = 1.2f + (float)GetRand(25) / 10.0f;
		}
	}

	for (int li = m_ParticlePool.GetActiveCount() - 1; li >= 0; li--)
	{
		const int idx = m_ParticlePool.GetActiveIndex(li);
		Particle& p = m_ParticlePool.GetSlots()[idx];

		p.x += p.vx;
		p.y += p.vy;
		p.z += p.vz;
		p.vx *= 0.94f;
		p.vy *= 0.94f;
		p.vz *= 0.94f;
		p.life--;
		if (p.life <= 0)
			m_ParticlePool.Release(idx);
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
	SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
	const int starDrawMax = EFFECT_STAR_DRAW_MAX < STAR_MAX ? EFFECT_STAR_DRAW_MAX : STAR_MAX;
	for (int i = 0; i < starDrawMax; i++)
		DrawCircleAA((int)m_Stars[i].screenX, (int)m_Stars[i].screenY, 1.8f, 2, m_Stars[i].color, TRUE);

	if (m_BombActive)
	{
		int alpha = 200 - (m_BombTimer * 4);
		if (alpha < 0) alpha = 0;
		SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
		VECTOR center = VGet(m_BombX, m_BombY, m_BombZ);
		DrawSphere3D(center, m_BombRadius, 8, GetColor(0, 255, 255), GetColor(0, 255, 255), FALSE);
	}

	int lastAlpha = -1;
	int particlesDrawn = 0;
	for (int li = 0; li < m_ParticlePool.GetActiveCount() && particlesDrawn < EFFECT_MAX_DRAW_PARTICLES; li++)
	{
		const Particle& p = m_ParticlePool.GetSlots()[m_ParticlePool.GetActiveIndex(li)];

		float lifeRatio = (float)p.life / (float)p.maxLife;
		int alpha = (int)(255.0f * lifeRatio);
		if (alpha != lastAlpha)
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			lastAlpha = alpha;
		}

		VECTOR sp = ConvWorldPosToScreenPos(VGet(p.x, p.y, p.z));
		if (sp.z < 0.0f || sp.z > 1.0f)
			continue;
		int r = (int)(p.radius * (1.5f - sp.z * 0.35f));
		if (r < 2) r = 2;
		DrawCircle((int)sp.x, (int)sp.y, r, p.color, TRUE);
		particlesDrawn++;
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

void Effect::AddExplosion(float x, float y, float z, unsigned int color, int count)
{
	if (count <= 0)
		count = EXPLOSION_PARTICLE_COUNT;

	int spawned = 0;
	while (spawned < count)
	{
		Particle* p = m_ParticlePool.Acquire();
		if (p == nullptr)
			break;

		p->x = x;
		p->y = y;
		p->z = z;

		float phi = ((float)GetRand(360) * 3.14159f) / 180.0f;
		float theta = ((float)GetRand(180) * 3.14159f) / 180.0f;
		float speed = 3.5f + (float)GetRand(60) / 10.0f;

		p->vx = sinf(theta) * cosf(phi) * speed;
		p->vy = sinf(theta) * sinf(phi) * speed * 0.2f;
		p->vz = cosf(theta) * speed;

		p->radius = 2.5f + (float)GetRand(25) / 10.0f;
		p->maxLife = 25 + GetRand(20);
		p->life = p->maxLife;
		p->color = color;
		p->active = true;
		spawned++;
	}
}

void Effect::AddGrazeSpark(float x, float y, float z)
{
	int spawned = 0;
	const int count = GRAZE_SPARK_COUNT;
	while (spawned < count)
	{
		Particle* p = m_ParticlePool.Acquire();
		if (p == nullptr)
			break;

		p->x = x;
		p->y = y;
		p->z = z;
		p->vx = (float)(GetRand(80) - 40) / 10.0f;
		p->vy = (float)(GetRand(10) - 5) / 10.0f;
		p->vz = -2.0f - (float)GetRand(40) / 10.0f;
		p->radius = 1.5f + (float)GetRand(10) / 10.0f;
		p->maxLife = 10 + GetRand(10);
		p->life = p->maxLife;
		p->color = GetColor(0, 255, 255);
		p->active = true;
		spawned++;
	}
}

void Effect::AddItemSparkle(float x, float y, float z, unsigned int color)
{
	int spawned = 0;
	while (spawned < 3)
	{
		Particle* p = m_ParticlePool.Acquire();
		if (p == nullptr)
			break;

		p->x = x;
		p->y = y;
		p->z = z;
		p->vx = (float)(GetRand(60) - 30) / 10.0f;
		p->vy = (float)(GetRand(40) - 20) / 10.0f;
		p->vz = (float)(GetRand(60) - 30) / 10.0f;
		p->radius = 1.0f + (float)GetRand(8) / 10.0f;
		p->maxLife = 8 + GetRand(8);
		p->life = p->maxLife;
		p->color = color;
		p->active = true;
		spawned++;
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
