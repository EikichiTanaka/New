#pragma once
#include "Game/Bullet.h"

// �Œ�z��{�󂫃C���f�b�N�X�X�^�b�N�{�A�N�e�B�u���X�g�i���t���[���̑S�����E�[���������������j
template<int Capacity>
class BulletPool
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

	Bullet* GetSlots() { return m_Slots; }
	const Bullet* GetSlots() const { return m_Slots; }
	int GetActiveCount() const { return m_ActiveCount; }

	Bullet* Acquire()
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

	void ReleaseAll()
	{
		while (m_ActiveCount > 0)
			Release(m_ActiveList[m_ActiveCount - 1]);
	}

	int GetActiveIndex(int listPos) const { return m_ActiveList[listPos]; }
	const int* GetActiveList() const { return m_ActiveList; }

private:
	Bullet m_Slots[Capacity];
	int m_FreeStack[Capacity];
	int m_ActiveList[Capacity];
	int m_FreeCount;
	int m_ActiveCount;
};
