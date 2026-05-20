#pragma once

#include "GameConfig.h"

// Records paddle history and draws ghost paddle(s) that reflect the ball
class GhostRecorder
{
public:
	void Init(Difficulty diff);
	void Record(float centerX, int width);

	// Main ghost used for collision
	void GetMainGhost(float& l, float& r, float& t, float& b, bool& active) const;

	void Draw() const;

private:
	void GetGhostAtDelay(int delayFrames, float& l, float& r, float& t, float& b, bool& active) const;

	float m_PosX[GHOST_HISTORY_MAX] = {};
	int m_Width[GHOST_HISTORY_MAX] = {};
	int m_Idx = 0;
	int m_Count = 0;
	int m_Delay = GHOST_DELAY_NORMAL;
};
