#pragma once

#include "GameConfig.h"

enum class BlockHitSide
{
	Top,
	Bottom,
	Left,
	Right
};

class GameEffect;

class BlockManager
{
public:
	void Init(Difficulty diff);
	void Draw() const;
	bool HitBall(float bx, float by, float br, int& outScore, BlockHitSide& outSide, GameEffect& effect, int& combo);
	void Replenish(Difficulty diff, GameEffect& effect);
	int GetRemain() const;
	int GetTotalAtStart() const { return m_TotalStart; }
	bool IsAllClear() const;

private:
	int m_Map[BLOCK_ROWS][BLOCK_COLS] = {};
	int m_Hp[BLOCK_ROWS][BLOCK_COLS] = {};
	int m_TotalStart = 0;
};
