#pragma once

struct Particle
{
	float x, y, z;
	float vx, vy, vz;
	int life;
	int maxLife;
	unsigned int color;
	float radius;
	bool active;
};

template<int Capacity>
class ParticlePool
{
public:
	void Init()
	{
		m_FreeCount = Capacity;
		m_ActiveCount = 0;
		for (int i = 0; i < Capacity; i++)
		{
			m_Slots[i].active = false;
			m_FreeStack[i] = i;
		}
	}

	Particle* GetSlots() { return m_Slots; }
	const Particle* GetSlots() const { return m_Slots; }
	int GetActiveCount() const { return m_ActiveCount; }

	Particle* Acquire()
	{
		if (m_FreeCount <= 0)
			return nullptr;

		const int idx = m_FreeStack[--m_FreeCount];
		m_ActiveList[m_ActiveCount++] = idx;
		return &m_Slots[idx];
	}

	void Release(int idx)
	{
		if (idx < 0 || idx >= Capacity)
			return;

		m_Slots[idx].active = false;

		for (int i = 0; i < m_ActiveCount; i++)
		{
			if (m_ActiveList[i] == idx)
			{
				m_ActiveList[i] = m_ActiveList[--m_ActiveCount];
				break;
			}
		}
		m_FreeStack[m_FreeCount++] = idx;
	}

	int GetActiveIndex(int listPos) const { return m_ActiveList[listPos]; }

private:
	Particle m_Slots[Capacity];
	int m_FreeStack[Capacity];
	int m_ActiveList[Capacity];
	int m_FreeCount;
	int m_ActiveCount;
};
