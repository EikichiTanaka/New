#pragma once

#include "GameConfig.h"
#include "Game/BlockManager.h"

class Paddle;

class Ball
{
public:
	void Init(Difficulty diff);
	void ResetOnPaddle(const Paddle& pad);
	void Launch();
	bool Update(const Paddle& pad, float gL, float gR, float gT, float gB, bool ghostOn, bool smashActive);
	void Draw() const;
	void ReflectBlock(BlockHitSide side);
	void AddSpeedOnPaddleHit(bool isSmash);
	void SetSuperBall(bool isSuper);
	bool IsSuperBall() const { return m_IsSuperBall; }

	bool IsLaunched() const { return m_On; }
	bool IsOutBottom() const;
	float GetX() const { return m_X; }
	float GetY() const { return m_Y; }
	float GetVY() const { return m_VY; }

private:
	bool HitPad(float l, float r, float t, float b, float cx);
	bool HitGhost(float l, float r, float t, float b);

	struct TrailNode {
		float x, y;
	};
	TrailNode m_Trail[TRAIL_MAX] = {};
	int m_TrailIdx = 0;

	float m_X = 0, m_Y = 0, m_VX = 0, m_VY = 0;
	float m_Spd = BALL_SPEED_NORMAL;
	bool m_On = false;
	bool m_IsSuperBall = false;
};
