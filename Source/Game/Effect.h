#pragma once
#include "GameConfig.h"
#include "Game/ParticlePool.h"

struct Star
{
	float screenX;
	float screenY;
	float speed;
	unsigned int color;
};

class Effect
{
public:
	void Init();
	void Update();
	void Draw() const;

	void AddExplosion(float x, float y, float z, unsigned int color, int count = 0);
	void AddGrazeSpark(float x, float y, float z);
	void AddItemSparkle(float x, float y, float z, unsigned int color);
	void TriggerBombShockwave(float x, float y, float z);

	float GetBombShockwaveRadius() const { return m_BombActive ? m_BombRadius : 0.0f; }
	bool IsBombActive() const { return m_BombActive; }

private:
	ParticlePool<PARTICLE_MAX> m_ParticlePool;

	static constexpr int STAR_MAX = 32;
	Star m_Stars[STAR_MAX];

	bool m_BombActive;
	float m_BombX, m_BombY, m_BombZ;
	float m_BombRadius;
	int m_BombTimer;
};
