#include "Game/GameEffect.h"
#include "GameConfig.h"
#include "DxLib.h"

void GameEffect::Clear()
{
	for (int i = 0; i < PARTICLE_MAX; i++)
		m_Particles[i].active = false;
	for (int i = 0; i < POPUP_MAX; i++)
		m_Popups[i].active = false;
	m_FlashTimer = 0;
	m_ShakeTimer = 0;
	m_ShakeIntensity = 0;
	m_GlitchTimer = 0;
}

void GameEffect::AddBlockBreak(float x, float y, bool isSuperBall)
{
	int count = isSuperBall ? 30 : 10;
	for (int i = 0; i < count; i++)
	{
		for (int j = 0; j < PARTICLE_MAX; j++)
		{
			if (m_Particles[j].active) continue;
			m_Particles[j].active = true;
			m_Particles[j].x = x;
			m_Particles[j].y = y;
			
			float speedMul = isSuperBall ? 1.5f : 1.0f;
			m_Particles[j].vx = (float)((GetRand(100) - 50) / 10.0f) * speedMul;
			m_Particles[j].vy = (float)((GetRand(100) - 70) / 10.0f) * speedMul;
			
			m_Particles[j].life = 20 + GetRand(15);
			if (isSuperBall)
				m_Particles[j].color = GetColor(255, 100 + GetRand(100), 0); // Fire colors
			else
				m_Particles[j].color = GetColor(0, 255, 120 + GetRand(100));
			break;
		}
	}
}

void GameEffect::AddScorePopup(float x, float y, int score, int combo)
{
	for (int i = 0; i < POPUP_MAX; i++)
	{
		if (m_Popups[i].active) continue;
		m_Popups[i].active = true;
		m_Popups[i].x = x;
		m_Popups[i].y = y - 10.0f;
		m_Popups[i].vy = -1.5f;
		m_Popups[i].life = 45;
		m_Popups[i].score = score;
		m_Popups[i].combo = combo;
		break;
	}
}

void GameEffect::AddLifeLostFlash()
{
	m_FlashTimer = 18;
	TriggerShake(15, 20);
}

void GameEffect::TriggerShake(int intensity, int durationFrames)
{
	if (intensity > m_ShakeIntensity || m_ShakeTimer == 0)
	{
		m_ShakeIntensity = intensity;
		m_ShakeTimer = durationFrames;
	}
}

void GameEffect::TriggerGlitch(int durationFrames)
{
	m_GlitchTimer = durationFrames;
	TriggerShake(25, durationFrames); // Also shake heavily
}

void GameEffect::Update()
{
	if (m_FlashTimer > 0) m_FlashTimer--;
	if (m_GlitchTimer > 0) m_GlitchTimer--;
	if (m_ShakeTimer > 0)
	{
		m_ShakeTimer--;
		if (m_ShakeTimer == 0) m_ShakeIntensity = 0;
	}

	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		if (!m_Particles[i].active) continue;
		m_Particles[i].x += m_Particles[i].vx;
		m_Particles[i].y += m_Particles[i].vy;
		m_Particles[i].vy += 0.4f; // Gravity
		
		m_Particles[i].life--;
		if (m_Particles[i].life <= 0)
			m_Particles[i].active = false;
	}

	for (int i = 0; i < POPUP_MAX; i++)
	{
		if (!m_Popups[i].active) continue;
		m_Popups[i].y += m_Popups[i].vy;
		m_Popups[i].vy *= 0.95f; // Slow down over time
		m_Popups[i].life--;
		if (m_Popups[i].life <= 0)
			m_Popups[i].active = false;
	}
}

void GameEffect::Draw() const
{
	int offsetX = 0;
	int offsetY = 0;
	if (m_ShakeTimer > 0)
	{
		int intensity = m_ShakeIntensity;
		if (m_GlitchTimer > 0) intensity += 10; // Extra heavy jitter on glitch
		offsetX = GetRand(intensity * 2) - intensity;
		offsetY = GetRand(intensity * 2) - intensity;
	}

	if (m_FlashTimer > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
		DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(255, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	// Cyber Screen Glitch horizontal neon bands
	if (m_GlitchTimer > 0)
	{
		SetDrawBlendMode(DX_BLENDMODE_ADD, 120 + GetRand(60));
		int strips = 2 + GetRand(3);
		for (int s = 0; s < strips; s++)
		{
			int h = 4 + GetRand(20);
			int y = GetRand(SCREEN_HEIGHT);
			int color = (GetRand(1) == 0) ? GetColor(0, 255, 255) : GetColor(255, 0, 100);
			DrawBox(0, y, SCREEN_WIDTH, y + h, color, TRUE);
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	for (int i = 0; i < PARTICLE_MAX; i++)
	{
		if (!m_Particles[i].active) continue;
		int alpha = m_Particles[i].life * 12;
		if (alpha > 255) alpha = 255;
		
		int px = (int)(m_Particles[i].x + offsetX);
		int py = (int)(m_Particles[i].y + offsetY);
		
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawCircle(px, py, 3, m_Particles[i].color, TRUE);
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	for (int i = 0; i < POPUP_MAX; i++)
	{
		if (!m_Popups[i].active) continue;
		int alpha = m_Popups[i].life * 8;
		if (alpha > 255) alpha = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		
		int x = (int)(m_Popups[i].x + offsetX);
		int y = (int)(m_Popups[i].y + offsetY);
		
		if (m_Popups[i].combo > 1)
		{
			DrawFormatString(x - 20, y - 15, GetColor(255, 200, 0), "COMBO x%d!", m_Popups[i].combo);
			DrawFormatString(x - 10, y, GetColor(0, 255, 100), "+%d", m_Popups[i].score);
		}
		else
		{
			DrawFormatString(x - 10, y, GetColor(255, 255, 255), "+%d", m_Popups[i].score);
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}
