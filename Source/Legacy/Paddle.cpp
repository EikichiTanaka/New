#include "Game/Paddle.h"
#include "DxLib.h"

void Paddle::Init(Difficulty diff)
{
	switch (diff)
	{
	case Difficulty::Easy:   m_W = PADDLE_WIDTH_EASY; break;
	case Difficulty::Hard:   m_W = PADDLE_WIDTH_HARD; break;
	default:                 m_W = PADDLE_WIDTH_NORMAL; break;
	}
	m_X = (FIELD_LEFT + FIELD_RIGHT) * 0.5f - m_W * 0.5f;
	m_SmashFlashTimer = 0;
}

void Paddle::Update()
{
	if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_A)) m_X -= PADDLE_SPEED;
	if (CheckHitKey(KEY_INPUT_RIGHT) || CheckHitKey(KEY_INPUT_D)) m_X += PADDLE_SPEED;

	float minX = (float)FIELD_LEFT;
	float maxX = (float)(FIELD_RIGHT - m_W);
	if (m_X < minX) m_X = minX;
	if (m_X > maxX) m_X = maxX;
	
	if (m_SmashFlashTimer > 0) m_SmashFlashTimer--;
}

void Paddle::Draw() const
{
	int x1 = (int)m_X;
	int y1 = PADDLE_Y;
	int x2 = x1 + m_W;
	int y2 = y1 + PADDLE_HEIGHT;

	if (m_SmashFlashTimer > 0)
	{
		DrawBox(x1, y1, x2, y2, GetColor(255, 255, 100), TRUE);
		DrawBox(x1, y1, x2, y2, GetColor(255, 255, 255), FALSE);
	}
	else
	{
		// Neon green/cyan cyber command bar
		DrawBox(x1, y1, x2, y2, GetColor(0, 150, 100), TRUE);
		DrawBox(x1, y1, x2, y2, GetColor(0, 255, 180), FALSE);
		
		// Inner energy core line
		DrawBox(x1 + 6, y1 + 4, x2 - 6, y2 - 4, GetColor(220, 255, 255), FALSE);
	}
}

void Paddle::TriggerSmashEffect()
{
	m_SmashFlashTimer = 15;
}

void Paddle::SetCenterX(float cx)
{
	m_X = cx - m_W * 0.5f;
	float minX = (float)FIELD_LEFT;
	float maxX = (float)(FIELD_RIGHT - m_W);
	if (m_X < minX) m_X = minX;
	if (m_X > maxX) m_X = maxX;
}
