#include "Game/Ball.h"
#include "Game/Paddle.h"
#include "Common/SoundSynth.h"
#include "DxLib.h"
#include <cmath>

void Ball::Init(Difficulty diff)
{
	switch (diff)
	{
	case Difficulty::Easy:   m_Spd = BALL_SPEED_EASY; break;
	case Difficulty::Hard:   m_Spd = BALL_SPEED_HARD; break;
	default:                 m_Spd = BALL_SPEED_NORMAL; break;
	}
	m_On = false;
	m_IsSuperBall = false;
	for (int i = 0; i < TRAIL_MAX; i++) m_Trail[i] = { 0, 0 };
}

void Ball::ResetOnPaddle(const Paddle& pad)
{
	m_On = false;
	m_X = pad.GetCenterX();
	m_Y = pad.GetTop() - BALL_RADIUS - 2.f;
	m_VX = m_VY = 0;
}

void Ball::Launch()
{
	if (m_On) return;
	m_On = true;
	m_VX = m_Spd * 0.5f;
	m_VY = -m_Spd;
}

bool Ball::Update(const Paddle& pad, float gL, float gR, float gT, float gB, bool ghostOn, bool smashActive)
{
	bool smashSuccess = false;
	if (!m_On)
	{
		m_X = pad.GetCenterX();
		m_Y = pad.GetTop() - BALL_RADIUS - 2.f;
		return false;
	}

	m_Trail[m_TrailIdx] = { m_X, m_Y };
	m_TrailIdx = (m_TrailIdx + 1) % TRAIL_MAX;

	m_X += m_VX;
	m_Y += m_VY;

	if (m_X - BALL_RADIUS < FIELD_LEFT)  { m_X = FIELD_LEFT + BALL_RADIUS;  m_VX = fabsf(m_VX); }
	if (m_X + BALL_RADIUS > FIELD_RIGHT) { m_X = FIELD_RIGHT - BALL_RADIUS; m_VX = -fabsf(m_VX); }
	if (m_Y - BALL_RADIUS < FIELD_TOP)   { m_Y = FIELD_TOP + BALL_RADIUS;   m_VY = fabsf(m_VY); }

	if (m_VY > 0)
	{
		if (HitPad(pad.GetLeft(), pad.GetRight(), pad.GetTop(), pad.GetBottom(), pad.GetCenterX()))
		{
			AddSpeedOnPaddleHit(smashActive);
			if (smashActive)
			{
				m_IsSuperBall = true;
				smashSuccess = true;
				SoundSynth::PlaySmash();
			}
			else
			{
				m_IsSuperBall = false; // Reset Super Ball when bouncing normally!
				SoundSynth::PlayPaddleBounce();
			}
		}
	}

	if (ghostOn)
	{
		HitGhost(gL, gR, gT, gB);
	}
	
	return smashSuccess;
}

bool Ball::HitPad(float l, float r, float t, float b, float cx)
{
	if (m_Y + BALL_RADIUS < t || m_Y - BALL_RADIUS > b) return false;
	if (m_X + BALL_RADIUS < l || m_X - BALL_RADIUS > r) return false;

	m_Y = t - BALL_RADIUS - 1.f;
	m_VY = -fabsf(m_VY);
	float hit = (m_X - cx) / ((r - l) * 0.5f);
	m_VX = hit * m_Spd * 1.2f;

	float len = sqrtf(m_VX * m_VX + m_VY * m_VY);
	if (len > 0.01f) { m_VX = m_VX / len * m_Spd; m_VY = m_VY / len * m_Spd; }
	return true;
}

bool Ball::HitGhost(float l, float r, float t, float b)
{
	if (m_Y + BALL_RADIUS < t || m_Y - BALL_RADIUS > b) return false;
	if (m_X + BALL_RADIUS < l || m_X - BALL_RADIUS > r) return false;

	float overlapL = (m_X + BALL_RADIUS) - l;
	float overlapR = r - (m_X - BALL_RADIUS);
	float overlapT = (m_Y + BALL_RADIUS) - t;
	float overlapB = b - (m_Y - BALL_RADIUS);

	float minOverlap = overlapL;
	if (overlapR < minOverlap) minOverlap = overlapR;
	if (overlapT < minOverlap) minOverlap = overlapT;
	if (overlapB < minOverlap) minOverlap = overlapB;

	// Only obstruct the ball when it is moving UP towards the blocks
	if (minOverlap == overlapB && m_VY < 0)
	{
		m_Y = b + BALL_RADIUS + 1.f;
		m_VY = fabsf(m_VY);
		
		// Add some chaos to the X velocity to make it harder
		float cx = (l + r) * 0.5f;
		float hit = (m_X - cx) / ((r - l) * 0.5f);
		m_VX += hit * 2.5f;
		float len = sqrtf(m_VX * m_VX + m_VY * m_VY);
		if (len > 0.01f) { m_VX = m_VX / len * m_Spd; m_VY = m_VY / len * m_Spd; }
		return true;
	}
	else if (m_VY < 0 && (minOverlap == overlapL || minOverlap == overlapR))
	{
		m_X = (minOverlap == overlapL) ? l - BALL_RADIUS - 1.f : r + BALL_RADIUS + 1.f;
		m_VX = (minOverlap == overlapL) ? -fabsf(m_VX) : fabsf(m_VX);
		return true;
	}
	
	return false;
}

void Ball::AddSpeedOnPaddleHit(bool isSmash)
{
	if (isSmash)
	{
		m_Spd = BALL_SPEED_MAX;
	}
	else
	{
		m_Spd += 0.08f;
		if (m_Spd > BALL_SPEED_MAX) m_Spd = BALL_SPEED_MAX;
	}
	float len = sqrtf(m_VX * m_VX + m_VY * m_VY);
	if (len > 0.01f) { m_VX = m_VX / len * m_Spd; m_VY = m_VY / len * m_Spd; }
}

void Ball::SetSuperBall(bool isSuper)
{
	m_IsSuperBall = isSuper;
}

void Ball::ReflectBlock(BlockHitSide side)
{
	if (m_IsSuperBall) return; // Super ball pierces blocks!

	switch (side)
	{
	case BlockHitSide::Top:    m_VY = -fabsf(m_VY); break;
	case BlockHitSide::Bottom: m_VY = fabsf(m_VY);  break;
	case BlockHitSide::Left:   m_VX = -fabsf(m_VX); break;
	case BlockHitSide::Right:  m_VX = fabsf(m_VX);  break;
	}
}

void Ball::Draw() const
{
	// Draw Trail in 2D
	for (int i = 0; i < TRAIL_MAX; i++)
	{
		int idx = (m_TrailIdx - 1 - i + TRAIL_MAX) % TRAIL_MAX;
		if (m_Trail[idx].x == 0 && m_Trail[idx].y == 0) continue;
		
		int alpha = 200 - (i * (200 / TRAIL_MAX));
		float radius = BALL_RADIUS * (1.0f - (float)i / TRAIL_MAX);
		if (m_IsSuperBall) radius += 2.0f; // Thicker trail for super ball!
		
		int color = m_IsSuperBall ? GetColor(255, alpha, 0) : GetColor(0, alpha, alpha);
		
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawCircle((int)m_Trail[idx].x, (int)m_Trail[idx].y, (int)radius, color, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// Draw Ball in 2D
	int bx = (int)m_X;
	int by = (int)m_Y;

	if (m_IsSuperBall)
	{
		// Blazing orange/yellow plasma ball
		float pulseRadius = BALL_RADIUS + 3.0f + sinf((float)GetTickCount() * 0.02f) * 2.0f;
		SetDrawBlendMode(DX_BLENDMODE_ADD, 180);
		DrawCircle(bx, by, (int)pulseRadius, GetColor(255, 50, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawCircle(bx, by, (int)(BALL_RADIUS + 1), GetColor(255, 200, 0), TRUE);
	}
	else
	{
		// Glowing cyber cyan circle
		DrawCircle(bx, by, (int)BALL_RADIUS, GetColor(0, 180, 255), TRUE);
		DrawCircle(bx, by, (int)BALL_RADIUS + 1, GetColor(220, 255, 255), FALSE);
	}
}

bool Ball::IsOutBottom() const
{
	return m_Y - BALL_RADIUS > FIELD_BOTTOM;
}
