#include "Scene/DifficultySelectScene.h"
#include "Manager/SceneManager.h"
#include "Common/GameData.h"
#include "Common/KeyHelper.h"
#include "GameConfig.h"
#include "DxLib.h"
#include <math.h>

void DifficultySelectScene::Init()
{
	m_SelectedIndex = static_cast<int>(g_GameData.difficulty);
	m_AnimTimer = 0;
	m_StartDelayTimer = 0;
}

void DifficultySelectScene::Update()
{
	m_AnimTimer++;

	if (m_StartDelayTimer > 0)
	{
		m_StartDelayTimer++;
		if (m_StartDelayTimer > 20) // 20 frames delay before starting
		{
			g_GameData.difficulty = static_cast<Difficulty>(m_SelectedIndex);
			SceneManager::GetInstance().RequestChangeScene(SceneID::Game);
		}
		return; // Ignore other inputs during startup flash
	}

	if (KeyHelper::IsTrigger(KEY_INPUT_UP))
	{
		m_SelectedIndex--;
		if (m_SelectedIndex < 0) m_SelectedIndex = 2;
	}
	if (KeyHelper::IsTrigger(KEY_INPUT_DOWN))
	{
		m_SelectedIndex++;
		if (m_SelectedIndex > 2) m_SelectedIndex = 0;
	}
	if (KeyHelper::IsTrigger(KEY_INPUT_1)) m_SelectedIndex = 0;
	if (KeyHelper::IsTrigger(KEY_INPUT_2)) m_SelectedIndex = 1;
	if (KeyHelper::IsTrigger(KEY_INPUT_3)) m_SelectedIndex = 2;

	if (KeyHelper::IsLaunchTrigger())
	{
		m_StartDelayTimer = 1;
	}
	if (KeyHelper::IsTrigger(KEY_INPUT_X) || KeyHelper::IsTrigger(KEY_INPUT_ESCAPE))
	{
		SceneManager::GetInstance().RequestChangeScene(SceneID::Title);
	}
}

void DifficultySelectScene::Draw()
{
	SetBackgroundColor(5, 10, 15);
	
	// Draw scrolling cyber grid
	SetDrawBlendMode(DX_BLENDMODE_ADD, 60);
	int gridOffset = m_AnimTimer % 40;
	int maxGridX = SCREEN_WIDTH / 40 + 1;
	int maxGridY = SCREEN_HEIGHT / 40 + 1;
	for (int i = -1; i <= maxGridX; i++)
	{
		DrawLine(i * 40 + gridOffset, 0, i * 40 + gridOffset, SCREEN_HEIGHT, GetColor(0, 100, 255));
	}
	for (int i = -1; i <= maxGridY; i++)
	{
		DrawLine(0, i * 40 + gridOffset, SCREEN_WIDTH, i * 40 + gridOffset, GetColor(0, 100, 255));
	}
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);

	int centerX = SCREEN_WIDTH / 2;
	int centerY = SCREEN_HEIGHT / 2;

	DrawFormatString(centerX - 100, centerY - 280, GetColor(0, 255, 200), "=== SELECT DIFFICULTY ===");

	const char* titles[3] = { "EASY", "NORMAL", "HARD" };
	const char* specs[3] = {
		"SYS: WIDE PADDLE | GHOST: SLOW",
		"SYS: STANDARD | GHOST: NORMAL",
		"SYS: FAST BALL, 2-HIT BLOCKS | GHOST: QUICK"
	};

	int startY = centerY - 140;
	for (int i = 0; i < 3; i++)
	{
		int yPos = startY + i * 110;
		bool isSelected = (i == m_SelectedIndex);
		int boxLeft = centerX - 350;
		int boxRight = centerX + 350;
		
		if (isSelected)
		{
			// Flashing effect when started
			if (m_StartDelayTimer > 0 && (m_StartDelayTimer % 4 < 2)) continue;

			int alpha = 150 + (int)(sinf(m_AnimTimer * 0.15f) * 105);
			SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
			DrawBox(boxLeft, yPos - 10, boxRight, yPos + 70, GetColor(0, 50, 150), TRUE);
			DrawFormatString(boxLeft + 30, yPos, GetColor(255, 255, 0), "> [ %d ] %s <", i + 1, titles[i]);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawFormatString(boxLeft + 50, yPos + 35, GetColor(255, 200, 0), "%s", specs[i]);
		}
		else
		{
			DrawFormatString(boxLeft + 30, yPos, GetColor(150, 150, 150), "  [ %d ] %s", i + 1, titles[i]);
			DrawFormatString(boxLeft + 50, yPos + 35, GetColor(100, 100, 100), "%s", specs[i]);
		}
	}

	if (m_StartDelayTimer == 0)
	{
		int pulseAlpha = 155 + (int)(sinf(m_AnimTimer * 0.1f) * 100);
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulseAlpha);
		DrawFormatString(centerX - 72, centerY + 240, GetColor(255, 255, 255), "ENTER/SPACE: Start");
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawFormatString(centerX - 72, centerY + 280, GetColor(150, 150, 150), "X / ESC    : Title");
	}
}

void DifficultySelectScene::Final()
{
}
