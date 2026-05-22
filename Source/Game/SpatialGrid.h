#pragma once
#include "GameConfig.h"
#include <cmath>

// XZ���ʂ̋ψ�O���b�h�i�e�E�G�̍L�擖���蔻��p�j
template<int Cols, int Rows, int MaxNodes>
class SpatialGridXZ
{
public:
	static constexpr int CellCount = Cols * Rows;

	void Clear()
	{
		for (int i = 0; i < CellCount; i++)
			m_Heads[i] = -1;
	}

	void Insert(int id, float x, float z)
	{
		if (id < 0 || id >= MaxNodes)
			return;

		const int cell = CellIndex(x, z);
		m_Next[id] = m_Heads[cell];
		m_Heads[cell] = id;
	}

	template<typename Func>
	void ForEachNear(float x, float z, float radius, Func&& fn) const
	{
		int cx0, cz0, cx1, cz1;
		CellRange(x, z, radius, cx0, cz0, cx1, cz1);

		for (int cz = cz0; cz <= cz1; cz++)
		{
			for (int cx = cx0; cx <= cx1; cx++)
			{
				const int cell = cz * Cols + cx;
				for (int id = m_Heads[cell]; id != -1; id = m_Next[id])
					fn(id);
			}
		}
	}

private:
	static constexpr float OriginX = -FIELD_HALF_W;
	static constexpr float OriginZ = -FIELD_HALF_D;
	static constexpr float CellW = FIELD_WIDTH / (float)Cols;
	static constexpr float CellD = FIELD_DEPTH / (float)Rows;

	static int ClampInt(int v, int lo, int hi)
	{
		if (v < lo) return lo;
		if (v > hi) return hi;
		return v;
	}

	static int CellIndex(float x, float z)
	{
		int cx = (int)((x - OriginX) / CellW);
		int cz = (int)((z - OriginZ) / CellD);
		cx = ClampInt(cx, 0, Cols - 1);
		cz = ClampInt(cz, 0, Rows - 1);
		return cz * Cols + cx;
	}

	static void CellRange(float x, float z, float radius, int& cx0, int& cz0, int& cx1, int& cz1)
	{
		cx0 = (int)std::floor((x - radius - OriginX) / CellW);
		cx1 = (int)std::floor((x + radius - OriginX) / CellW);
		cz0 = (int)std::floor((z - radius - OriginZ) / CellD);
		cz1 = (int)std::floor((z + radius - OriginZ) / CellD);
		cx0 = ClampInt(cx0, 0, Cols - 1);
		cx1 = ClampInt(cx1, 0, Cols - 1);
		cz0 = ClampInt(cz0, 0, Rows - 1);
		cz1 = ClampInt(cz1, 0, Rows - 1);
	}

	int m_Heads[CellCount];
	int m_Next[MaxNodes];
};
