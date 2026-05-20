#include "Scene/TitleScene.h"
#include "Manager/SceneManager.h"
#include "Common/KeyHelper.h"
#include "DxLib.h"
#include "GameConfig.h"
#include <math.h>

void TitleScene::Init()
{
	m_Timer = 0;
	for (int i = 0; i < MAX_RAIN; i++)
	{
		m_Rain[i].x = (float)(GetRand(SCREEN_WIDTH / 10) * 10);
		m_Rain[i].y = (float)(GetRand(SCREEN_HEIGHT));
		m_Rain[i].speed = 2.0f + GetRand(5);
		m_Rain[i].length = 5 + GetRand(10);
	}
}

void TitleScene::Update()
{
	m_Timer++;
	for (int i = 0; i < MAX_RAIN; i++)
	{
		m_Rain[i].y += m_Rain[i].speed;
		if (m_Rain[i].y > SCREEN_HEIGHT + 100)
		{
			m_Rain[i].y = -100.0f;
			m_Rain[i].x = (float)(GetRand(SCREEN_WIDTH / 10) * 10);
		}
	}

	if (KeyHelper::IsLaunchTrigger())
		SceneManager::GetInstance().RequestChangeScene(SceneID::DifficultySelect);
}
void TitleScene::Draw()
{
	SetBackgroundColor(5, 10, 5);

	// Draw Matrix Rain
	SetDrawBlendMode(DX_BLENDMODE_ADD, 120);
	for (int i = 0; i < MAX_RAIN; i++)
	{
		for (int j = 0; j < m_Rain[i].length; j++)
		{
			float py = m_Rain[i].y - j * 12;
			if (py > 0 && py < SCREEN_HEIGHT)
			{
				int alpha = 255 - j * (255 / m_Rain[i].length);
				SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
				DrawFormatString((int)m_Rain[i].x, (int)py, GetColor(0, 255, 100), "%d", GetRand(9));
			}
		}
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// Dummy Hacker Logs
	SetDrawBlendMode(DX_BLENDMODE_ADD, 80);
	DrawFormatString(20, 20 + (m_Timer % 200) / 5, GetColor(0, 200, 100), "[INFO] Connecting to Neuro-Link...");
	DrawFormatString(20, 40 + (m_Timer % 200) / 5, GetColor(0, 200, 100), "[WARN] Ghost entities detected.");
	DrawFormatString(20, 60 + (m_Timer % 200) / 5, GetColor(0, 200, 100), "[OK] System overrides ready.");
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	// Glitching Title Logo
	int centerX = SCREEN_WIDTH / 2;
	int centerY = SCREEN_HEIGHT / 2;
	int titleX = centerX - 80;
	int titleY = centerY - 120;
	int color = GetColor(0, 255, 80);
	if (GetRand(100) < 5) // 5% chance to glitch each frame
	{
		titleX += GetRand(10) - 5;
		titleY += GetRand(4) - 2;
		color = GetRand(2) == 0 ? GetColor(255, 0, 0) : GetColor(0, 255, 255);
	}
	DrawFormatString(titleX, titleY, color, "=== HACKER BRAIN ===");
	
	DrawFormatString(centerX - 112, titleY + 60, GetColor(150, 255, 150), "Block Breaker + Ghost Paddle");
	DrawFormatString(centerX - 148, titleY + 110, GetColor(200, 200, 200), "Your past movement blocks the ball...");
	
	// Pulsing Start Prompt
	int pulseAlpha = 155 + (int)(sinf(m_Timer * 0.1f) * 100);
	SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulseAlpha);
	DrawFormatString(centerX - 52, titleY + 230, GetColor(255, 255, 255), "ENTER : Start");
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawFormatString(centerX - 48, titleY + 270, GetColor(100, 100, 100), "ESC   : Quit");
}
void TitleScene::Final() {}
