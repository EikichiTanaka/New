#include "Game/BlockManager.h"
#include "Game/GameEffect.h"
#include "Common/SoundSynth.h"
#include "DxLib.h"
#include <math.h>

void BlockManager::Init(Difficulty diff)
{
	m_TotalStart = 0;
	int rows = BLOCK_ROWS;
	if (diff == Difficulty::Easy) rows = 5;

	for (int r = 0; r < BLOCK_ROWS; r++)
	{
		for (int c = 0; c < BLOCK_COLS; c++)
		{
			if (r < rows)
			{
				// 10% chance to spawn a Bomb block (not on the very bottom row to be safe)
				if (r > 0 && GetRand(100) < 12)
				{
					m_Map[r][c] = 2; // Bomb block
					m_Hp[r][c] = 1;
				}
				else
				{
					m_Map[r][c] = 1; // Normal block
					m_Hp[r][c] = (diff == Difficulty::Hard) ? 2 : 1;
				}
				m_TotalStart++;
			}
			else
			{
				m_Map[r][c] = 0;
				m_Hp[r][c] = 0;
			}
		}
	}
}

void BlockManager::Draw() const
{
	static int drawTimer = 0;
	drawTimer++;

	for (int r = 0; r < BLOCK_ROWS; r++)
	{
		for (int c = 0; c < BLOCK_COLS; c++)
		{
			if (!m_Map[r][c]) continue;

			int x1 = BLOCK_START_X + c * (BLOCK_WIDTH + BLOCK_GAP_X);
			int y1 = BLOCK_START_Y + r * (BLOCK_HEIGHT + BLOCK_GAP_Y);
			int x2 = x1 + BLOCK_WIDTH;
			int y2 = y1 + BLOCK_HEIGHT;
			
			if (m_Map[r][c] == 2) // Bomb Block
			{
				// Dark red filled block with bright red border
				DrawBox(x1, y1, x2, y2, GetColor(150, 0, 0), TRUE);
				DrawBox(x1, y1, x2, y2, GetColor(255, 50, 50), FALSE);
				
				// Pulsing orange/yellow warning core
				int pulse = 80 + (int)(sinf(drawTimer * 0.2f) * 60);
				DrawBox(x1 + 4, y1 + 4, x2 - 4, y2 - 4, GetColor(255, pulse, 0), TRUE);
			}
			else // Normal Block
			{
				int fill = GetColor(0, 140 + r * 15, 50 + c * 8);
				DrawBox(x1, y1, x2, y2, fill, TRUE);
				DrawBox(x1, y1, x2, y2, GetColor(0, 255, 100), FALSE);

				if (m_Hp[r][c] >= 2)
				{
					// Yellow glowing shield indicator circle
					int cx = (x1 + x2) / 2;
					int cy = (y1 + y2) / 2;
					DrawCircle(cx, cy, 5, GetColor(255, 255, 0), TRUE);
				}
			}
		}
	}
}

bool BlockManager::HitBall(float bx, float by, float br, int& outScore, BlockHitSide& outSide, GameEffect& effect, int& combo)
{
	outScore = 0;
	outSide = BlockHitSide::Top;

	for (int r = 0; r < BLOCK_ROWS; r++)
	{
		for (int c = 0; c < BLOCK_COLS; c++)
		{
			if (!m_Map[r][c]) continue;

			float l = (float)(BLOCK_START_X + c * (BLOCK_WIDTH + BLOCK_GAP_X));
			float t = (float)(BLOCK_START_Y + r * (BLOCK_HEIGHT + BLOCK_GAP_Y));
			float ri = l + BLOCK_WIDTH;
			float bo = t + BLOCK_HEIGHT;

			float nx = (bx < l) ? l : (bx > ri ? ri : bx);
			float ny = (by < t) ? t : (by > bo ? bo : by);
			float dx = bx - nx;
			float dy = by - ny;
			if (dx * dx + dy * dy > br * br) continue;

			float penL = (bx + br) - l;
			float penR = ri - (bx - br);
			float penT = (by + br) - t;
			float penB = bo - (by - br);

			float minPen = penT;
			outSide = BlockHitSide::Top;
			if (penB < minPen) { minPen = penB; outSide = BlockHitSide::Bottom; }
			if (penL < minPen) { minPen = penL; outSide = BlockHitSide::Left; }
			if (penR < minPen) { outSide = BlockHitSide::Right; }

			int rowBonus = (BLOCK_ROWS - r) * 10;
			
			if (m_Map[r][c] == 2) // Bomb block hit!
			{
				// Trigger visual explosion and shake
				effect.TriggerShake(20, 15);
				
				// Destroy self
				m_Map[r][c] = 0;
				m_Hp[r][c] = 0;
				outScore = SCORE_PER_BLOCK + rowBonus;
				
				// Neighborhood explosion!
				for (int nr = r - 1; nr <= r + 1; nr++)
				{
					for (int nc = c - 1; nc <= c + 1; nc++)
					{
						if (nr >= 0 && nr < BLOCK_ROWS && nc >= 0 && nc < BLOCK_COLS)
						{
							if (m_Map[nr][nc] > 0)
							{
								m_Map[nr][nc] = 0;
								m_Hp[nr][nc] = 0;
								
								combo++;
								int neighborBonus = (BLOCK_ROWS - nr) * 10;
								int reward = SCORE_PER_BLOCK + neighborBonus;
								outScore += reward;

								float cx = BLOCK_START_X + nc * (BLOCK_WIDTH + BLOCK_GAP_X) + BLOCK_WIDTH / 2.0f;
								float cy = BLOCK_START_Y + nr * (BLOCK_HEIGHT + BLOCK_GAP_Y) + BLOCK_HEIGHT / 2.0f;
								
								effect.AddBlockBreak(cx, cy, true);
								effect.AddScorePopup(cx, cy, reward, combo);
								
								SoundSynth::PlayBlockBreak(combo);
							}
						}
					}
				}
			}
			else // Normal block hit
			{
				m_Hp[r][c]--;
				if (m_Hp[r][c] <= 0)
				{
					m_Map[r][c] = 0;
					outScore = SCORE_PER_BLOCK + rowBonus;
				}
				else
				{
					outScore = SCORE_PER_BLOCK / 2;
				}
			}
			return true;
		}
	}
	return false;
}

int BlockManager::GetRemain() const
{
	int n = 0;
	for (int r = 0; r < BLOCK_ROWS; r++)
		for (int c = 0; c < BLOCK_COLS; c++)
			if (m_Map[r][c]) n++;
	return n;
}

bool BlockManager::IsAllClear() const
{
	return GetRemain() == 0;
}

void BlockManager::Replenish(Difficulty diff, GameEffect& effect)
{
	int rows = BLOCK_ROWS;
	if (diff == Difficulty::Easy) rows = 6;

	for (int r = 0; r < BLOCK_ROWS; r++)
	{
		for (int c = 0; c < BLOCK_COLS; c++)
		{
			if (r < rows && m_Map[r][c] == 0)
			{
				if (r > 0 && GetRand(100) < 12)
				{
					m_Map[r][c] = 2; // Bomb block
					m_Hp[r][c] = 1;
				}
				else
				{
					m_Map[r][c] = 1; // Normal block
					m_Hp[r][c] = (diff == Difficulty::Hard) ? 2 : 1;
				}

				// Spawn digital reload sparks
				float cx = BLOCK_START_X + c * (BLOCK_WIDTH + BLOCK_GAP_X) + BLOCK_WIDTH / 2.0f;
				float cy = BLOCK_START_Y + r * (BLOCK_HEIGHT + BLOCK_GAP_Y) + BLOCK_HEIGHT / 2.0f;
				effect.AddBlockBreak(cx, cy, false);
			}
		}
	}
}
