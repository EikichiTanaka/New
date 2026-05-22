#include "Scene/TitleScene.h"
#include "GameConfig.h"
#include "Common/KeyHelper.h"
#include "DxLib.h"

void TitleScene::Init()
{
	m_AnimTimer = 0;
}

SceneType TitleScene::Update()
{
	m_AnimTimer++;

	if (KeyHelper::IsCancelTrigger())
		return SceneType::Exit;

	if (KeyHelper::IsTrigger(KEY_INPUT_O))
		return SceneType::Options;

	if (KeyHelper::IsConfirmTrigger())
		return SceneType::DifficultySelect;

	return SceneType::None;
}

void TitleScene::Draw()
{
	int centerX = SCREEN_WIDTH / 2;
	int centerY = SCREEN_HEIGHT / 2;

	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / (float)SCREEN_HEIGHT;
		int r = (int)(8 * (1.0f - t));
		int g = (int)(4 * (1.0f - t));
		int b = (int)(40 + 20 * (1.0f - t));
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(r, g, b));
	}

	for (int i = 0; i < 60; i++)
	{
		int sx = (i * 137 + m_AnimTimer * 4) % SCREEN_WIDTH;
		int sy = (i * 53) % SCREEN_HEIGHT;
		int br = 80 + (i * 17) % 175;
		DrawPixel(sx, sy, GetColor(br, br, br));
	}

	SetFontSize(72);
	DrawFormatString(centerX - 240, centerY - 180, GetColor(255, 80, 120), "BULLET STORM");

	SetFontSize(28);
	DrawFormatString(centerX - 200, centerY - 90, GetColor(0, 230, 255), "― 超爽快３Ｄ弾幕シューティング ―");

	SetFontSize(22);
	DrawFormatString(centerX - 280, centerY - 30, GetColor(220, 220, 220), "敵弾の嵐をかいくぐり、超弾幕でねじ伏せろ！");

	if ((m_AnimTimer / 30) % 2 == 0)
	{
		SetFontSize(26);
		DrawFormatString(centerX - 200, centerY + 60, GetColor(255, 255, 0), ">>  ＥＮＴＥＲキーでスタート  <<");
	}

	SetFontSize(18);
	DrawFormatString(centerX - 220, centerY + 130, GetColor(180, 200, 255), "移動：方向キー / WASD    低速：Shift");
	DrawFormatString(centerX - 220, centerY + 155, GetColor(180, 200, 255), "ショット：Ｚ / Space     ボム：Ｘ / Ｂ");

	SetFontSize(16);
	DrawFormatString(centerX - 160, centerY + 200, GetColor(140, 140, 140), "O:オプション  ESC:終了");

	SetFontSize(20);
}
