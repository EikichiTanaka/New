#include "Scene/ResultScene.h"
#include "Manager/SceneManager.h"
#include "Common/GameData.h"
#include "Common/KeyHelper.h"
#include "GameConfig.h"
#include "DxLib.h"
#include <math.h>

void ResultScene::Init()
{
	m_TargetScore = g_GameData.score;
	m_DisplayScore = 0.0f;
	m_AnimTimer = 0;
	m_ShakeTimer = 0;
	
	if (m_TargetScore >= 8000 || g_GameData.maxCombo >= 20) m_Rank = 'S';
	else if (m_TargetScore >= 4000) m_Rank = 'A';
	else if (m_TargetScore >= 1500) m_Rank = 'B';
	else m_Rank = 'C';
}

void ResultScene::Update()
{
	m_AnimTimer++;
	if (m_ShakeTimer > 0) m_ShakeTimer--;
	
	if (m_DisplayScore < m_TargetScore)
	{
		m_DisplayScore += (m_TargetScore - m_DisplayScore) * 0.1f + 10.0f;
		if (m_DisplayScore >= m_TargetScore)
		{
			m_DisplayScore = (float)m_TargetScore;
			m_ShakeTimer = 20; // Trigger shake when score finishes
		}
	}

	if (KeyHelper::IsLaunchTrigger())
	{
		if (m_DisplayScore < m_TargetScore)
		{
			// Skip animation
			m_DisplayScore = (float)m_TargetScore;
			m_ShakeTimer = 20; // Trigger shake when skipped
		}
		else if (m_AnimTimer > 15) // Debounce exit slightly
		{
			SceneManager::GetInstance().RequestChangeScene(SceneID::Title);
		}
	}
}

void ResultScene::Draw()
{
	int offsetX = 0, offsetY = 0;
	if (m_ShakeTimer > 0)
	{
		offsetX = GetRand(m_ShakeTimer) - m_ShakeTimer / 2;
		offsetY = GetRand(m_ShakeTimer) - m_ShakeTimer / 2;
	}

	if (g_GameData.isClear)
	{
		SetBackgroundColor(0, 30, 10);
		// Draw some green cyber grid lines scaled to screen size
		SetDrawBlendMode(DX_BLENDMODE_ADD, 50);
		int rows = SCREEN_HEIGHT / 40 + 2;
		for (int i = 0; i < rows; i++)
		{
			DrawLine(0, i * 40 + (m_AnimTimer % 40), SCREEN_WIDTH, i * 40 + (m_AnimTimer % 40), GetColor(0, 255, 100));
		}
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
	else
	{
		SetBackgroundColor(30, 0, 5);
		// Draw red warning scanline scaled to screen size
		SetDrawBlendMode(DX_BLENDMODE_ADD, 80);
		int scanY = (m_AnimTimer * 5) % SCREEN_HEIGHT;
		DrawBox(0, scanY, SCREEN_WIDTH, scanY + 20, GetColor(255, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	const char* msg = "SESSION END";
	if (g_GameData.isClear)     msg = "CLEAR! Brain Data Purged";
	if (g_GameData.isStalemate) msg = "STALEMATE! Blocks Locked";
	if (g_GameData.isGameOver)  msg = "GAME OVER! Sync Failed";

	int centerX = SCREEN_WIDTH / 2;
	int centerY = SCREEN_HEIGHT / 2;

	DrawFormatString(centerX - 100 + offsetX, centerY - 280 + offsetY, GetColor(255, 200, 0), "=== BRAIN SYNC RESULT ===");
	
	int msgShift = (int)strlen(msg) * 4;
	DrawFormatString(centerX - msgShift + offsetX, centerY - 210 + offsetY, GetColor(0, 255, 160), "%s", msg);
	
	int statsX = centerX - 80;
	int statsY = centerY - 100;
	DrawFormatString(statsX + offsetX, statsY + 0 + offsetY, GetColor(255, 255, 255), "SCORE      : %d", (int)m_DisplayScore);
	DrawFormatString(statsX + offsetX, statsY + 40 + offsetY, GetColor(200, 200, 255), "DESTROYED  : %d", g_GameData.blocksBroken);
	DrawFormatString(statsX + offsetX, statsY + 80 + offsetY, GetColor(200, 200, 255), "MAX COMBO  : x%d", g_GameData.maxCombo);
	DrawFormatString(statsX + offsetX, statsY + 120 + offsetY, GetColor(200, 200, 255), "TIME       : %d sec", g_GameData.playTimeSec);
	DrawFormatString(statsX + offsetX, statsY + 160 + offsetY, GetColor(200, 200, 255), "LIVES LEFT : %d", g_GameData.livesLeft);
	
	if (m_DisplayScore >= m_TargetScore)
	{
		// Draw Giant Rank Letter
		int rankColor = GetColor(0, 255, 100);
		if (m_Rank == 'A') rankColor = GetColor(0, 150, 255);
		if (m_Rank == 'B') rankColor = GetColor(255, 200, 0);
		if (m_Rank == 'C') rankColor = GetColor(150, 150, 150);
		
		int fontSize = 140 + (m_ShakeTimer * 5); // Scale down effect when hitting
		int rankX = centerX - 320 + offsetX - fontSize / 3;
		int rankY = centerY - 80 + offsetY - fontSize / 3;
		
		SetDrawBlendMode(DX_BLENDMODE_ADD, 150 + m_ShakeTimer * 5);
		DrawFormatString(rankX, rankY, rankColor, "%c", m_Rank);
		DrawFormatString(rankX + 2, rankY + 2, GetColor(255,255,255), "%c", m_Rank);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

		// Pulse the text alpha to draw attention
		int alpha = 150 + (int)(sinf(m_AnimTimer * 0.1f) * 100);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
		DrawFormatString(centerX - 88 + offsetX, centerY + 240 + offsetY, GetColor(180, 180, 180), "ENTER : Back to Title");
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void ResultScene::Final() {}
