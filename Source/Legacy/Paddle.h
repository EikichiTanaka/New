#pragma once

#include "GameConfig.h"

class Paddle
{
public:
	void Init(Difficulty diff);
	void Update();
	void Draw() const;
	void SetCenterX(float cx);

	float GetCenterX() const { return m_X + m_W * 0.5f; }
	void TriggerSmashEffect();

	float GetLeft() const { return m_X; }
	float GetRight() const { return m_X + m_W; }
	float GetTop() const { return (float)PADDLE_Y; }
	float GetBottom() const { return (float)(PADDLE_Y + PADDLE_HEIGHT); }
	int GetWidth() const { return m_W; }

private:
	float m_X = 0;
	int m_W = PADDLE_WIDTH_NORMAL;
	int m_SmashFlashTimer = 0;
};
