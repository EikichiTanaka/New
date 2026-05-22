#include "Scene/OptionsScene.h"
#include "Common/GameOptions.h"
#include "Common/KeyHelper.h"
#include "Common/GameScreen.h"
#include "Common/UiDraw.h"
#include "GameConfig.h"
#include "DxLib.h"

static const int ROW_COUNT = 6;

void OptionsScene::Init()
{
	m_CursorRow = 0;
	m_NeedGraphicsApply = false;
}

SceneType OptionsScene::Update()
{
	if (KeyHelper::IsCancelTrigger())
	{
		GameOptionsSave();
		if (m_NeedGraphicsApply)
			GameOptionsApplyGraphics();
		else
			GameScreenSyncSize();
		GameOptionsApplyVolumes();
		return SceneType::Title;
	}

	if (KeyHelper::IsTrigger(KEY_INPUT_UP))
	{
		m_CursorRow--;
		if (m_CursorRow < 0) m_CursorRow = ROW_COUNT - 1;
	}
	if (KeyHelper::IsTrigger(KEY_INPUT_DOWN))
	{
		m_CursorRow++;
		if (m_CursorRow >= ROW_COUNT) m_CursorRow = 0;
	}

	const float stepVol = 0.05f;

	if (KeyHelper::IsTrigger(KEY_INPUT_LEFT) || KeyHelper::IsTrigger(KEY_INPUT_RIGHT))
	{
		int dir = KeyHelper::IsTrigger(KEY_INPUT_RIGHT) ? 1 : -1;
		switch (m_CursorRow)
		{
		case 0:
		{
			float v = g_Options.seVolume + dir * stepVol;
			if (v < 0.0f) v = 0.0f;
			if (v > 1.0f) v = 1.0f;
			g_Options.seVolume = v;
			break;
		}
		case 1:
		{
			float v = g_Options.bgmVolume + dir * stepVol;
			if (v < 0.0f) v = 0.0f;
			if (v > 1.0f) v = 1.0f;
			g_Options.bgmVolume = v;
			break;
		}
		case 2:
			g_Options.fullscreen = !g_Options.fullscreen;
			m_NeedGraphicsApply = true;
			break;
		case 3:
			if (dir > 0) { g_Options.resWidth = 1920; g_Options.resHeight = 1080; }
			else { g_Options.resWidth = 1280; g_Options.resHeight = 720; }
			m_NeedGraphicsApply = true;
			break;
		case 4:
			g_Options.hitStopEnabled = !g_Options.hitStopEnabled;
			break;
		default:
			break;
		}
		GameOptionsApplyVolumes();
	}

	return SceneType::None;
}

void OptionsScene::Draw()
{
	int cx = SCREEN_WIDTH / 2;

	for (int y = 0; y < SCREEN_HEIGHT; y += 2)
	{
		float t = (float)y / (float)SCREEN_HEIGHT;
		DrawLine(0, y, SCREEN_WIDTH, y, GetColor(8, 12, 28 + (int)(20 * (1 - t))));
	}

	const char* labels[] = {
		"効果音量",
		"BGM音量",
		"フルスクリーン",
		"解像度",
		"ヒットストップ",
		"（ESCで保存して戻る）"
	};

	SetFontSize(40);
	DrawTextUtf8(cx - 120, 40, GetColor(0, 230, 255), "オプション");

	char valBuf[64];
	for (int i = 0; i < ROW_COUNT; i++)
	{
		int rowY = 120 + i * 52;
		bool sel = (i == m_CursorRow);
		if (sel && i < 5)
		{
			SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
			DrawBox(cx - 380, rowY - 6, cx + 380, rowY + 36, GetColor(0, 60, 90), TRUE);
			SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			DrawTextUtf8(cx - 360, rowY, GetColor(255, 255, 0), ">");
		}

		SetFontSize(22);
		DrawTextUtf8(cx - 320, rowY, GetColor(200, 200, 220), labels[i]);

		if (i >= 5) continue;

		SetFontSize(24);
		unsigned int vc = sel ? GetColor(255, 255, 255) : GetColor(0, 255, 200);
		switch (i)
		{
		case 0: sprintf_s(valBuf, "%d%%", (int)(g_Options.seVolume * 100)); break;
		case 1: sprintf_s(valBuf, "%d%%", (int)(g_Options.bgmVolume * 100)); break;
		case 2: sprintf_s(valBuf, "%s", g_Options.fullscreen ? "ON" : "OFF"); break;
		case 3: sprintf_s(valBuf, "%dx%d", g_Options.resWidth, g_Options.resHeight); break;
		case 4: sprintf_s(valBuf, "%s", g_Options.hitStopEnabled ? "ON" : "OFF"); break;
		default: valBuf[0] = 0; break;
		}
		DrawFormatString(cx + 80, rowY, vc, "%s", valBuf);
	}

	if (m_NeedGraphicsApply)
	{
		SetFontSize(18);
		DrawTextUtf8(cx - 220, SCREEN_HEIGHT - 50, GetColor(255, 200, 100),
			"※ 画面設定は戻るときに反映されます");
	}

	SetFontSize(18);
	DrawTextUtf8(cx - 160, SCREEN_HEIGHT - 80, GetColor(160, 160, 180),
		"左右:変更  ESC:戻る");
}
