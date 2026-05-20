#include "Game/GhostRecorder.h"
#include "DxLib.h"

void GhostRecorder::Init(Difficulty diff)
{
	switch (diff)
	{
	case Difficulty::Easy:   m_Delay = GHOST_DELAY_EASY; break;
	case Difficulty::Hard:   m_Delay = GHOST_DELAY_HARD; break;
	default:                 m_Delay = GHOST_DELAY_NORMAL; break;
	}
	float cx = (FIELD_LEFT + FIELD_RIGHT) * 0.5f;
	for (int i = 0; i < GHOST_HISTORY_MAX; i++)
	{
		m_PosX[i] = cx;
		m_Width[i] = PADDLE_WIDTH_NORMAL;
	}
	m_Idx = 0;
	m_Count = 0;
}

void GhostRecorder::Record(float centerX, int width)
{
	m_PosX[m_Idx] = centerX;
	m_Width[m_Idx] = width;
	m_Idx = (m_Idx + 1) % GHOST_HISTORY_MAX;
	m_Count++;
}

void GhostRecorder::GetGhostAtDelay(int delayFrames, float& l, float& r, float& t, float& b, bool& active) const
{
	active = false;
	l = r = t = b = 0;
	if (m_Count <= delayFrames) return;

	int gi = (m_Idx - delayFrames + GHOST_HISTORY_MAX) % GHOST_HISTORY_MAX;
	float half = m_Width[gi] * 0.5f;
	l = m_PosX[gi] - half;
	r = m_PosX[gi] + half;
	t = (float)(FIELD_TOP + 320); 
	b = t + PADDLE_HEIGHT;
	active = true;
}

void GhostRecorder::GetMainGhost(float& l, float& r, float& t, float& b, bool& active) const
{
	GetGhostAtDelay(m_Delay, l, r, t, b, active);
}

void GhostRecorder::Draw() const
{
	int delays[GHOST_TRAIL_DRAW] = { m_Delay, m_Delay / 2, m_Delay / 4 };
	if (delays[1] < 8) delays[1] = 8;
	if (delays[2] < 4) delays[2] = 4;

	for (int i = GHOST_TRAIL_DRAW - 1; i >= 0; i--)
	{
		float l, r, t, b;
		bool on = false;
		GetGhostAtDelay(delays[i], l, r, t, b, on);
		if (!on) continue;

		int alpha = 50 + i * 35;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		int col = GetColor(120 + i * 30, 0, 200 + i * 15);
		DrawBox((int)l, (int)t, (int)r, (int)b, col, TRUE);
		DrawBox((int)l, (int)t, (int)r, (int)b, GetColor(255, 100, 255), FALSE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	float l, r, t, b;
	bool on = false;
	GetMainGhost(l, r, t, b, on);
	if (on)
	{
		int sx = (int)((l + r) * 0.5f) - 24;
		int sy = (int)t - 22;
		DrawFormatString(sx, sy, GetColor(255, 150, 255), "GHOST");
	}
}
