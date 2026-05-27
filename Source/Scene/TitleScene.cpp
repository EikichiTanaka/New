#include "Scene/TitleScene.h"
#include "GameConfig.h"
#include "Common/KeyHelper.h"
#include "Common/UiDraw.h"
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
	const int cx = SCREEN_WIDTH / 2;
	const int layoutShiftY = SCREEN_HEIGHT * 6 / 100;
	const int cy = SCREEN_HEIGHT / 2 + layoutShiftY;

	const char* logo = "BULLET STORM";
	const char* subtitle = u8"\u2015 \u8d85\u723d\u5feb\uff13\uff24\u5f3e\u5e55\u30b7\u30e5\u30fc\u30c6\u30a3\u30f3\u30b0 \u2015";
	const char* tagline = u8"\u6575\u5f3e\u306e\u5d50\u3092\u304b\u3044\u304f\u3050\u308a\u3001\u8d85\u5f3e\u5e55\u3067\u306d\u3058\u4f0f\u305b\u308d\uff01";
	const char* enterPrompt = u8">>\u3000\uff25\uff2e\uff34\uff25\uff32\u30ad\u30fc\u3067\u30b9\u30bf\u30fc\u30c8\u3000<<";
	const char* ctrlMove = u8"\u79fb\u52d5\uff1a\u65b9\u5411\u30ad\u30fc / WASD\u3000\u3000\u4f4e\u901f\uff1aShift";
	const char* ctrlShot = u8"\u30b7\u30e7\u30c3\u30c8\uff1a\uff3a / Space\u3000\u3000\u30dc\u30e0\uff1a\uff38 / \uff22";
	const char* footer = u8"O:\u30aa\u30d7\u30b7\u30e7\u30f3\u3000ESC:\u7d42\u4e86";

	const int gapSm = SCREEN_HEIGHT * 3 / 100;
	const int gapMd = SCREEN_HEIGHT * 4 / 100;
	const int gapLg = SCREEN_HEIGHT * 5 / 100;
	const int sepAfterLogo = 14;
	const int sepAfterLine = 18;

	SetFontSize(84);
	const int hLogo = GetTextDrawHeight(logo);
	SetFontSize(34);
	const int hSubtitle = GetTextDrawHeight(subtitle);
	SetFontSize(26);
	const int hTagline = GetTextDrawHeight(tagline);
	SetFontSize(32);
	const int hEnter = GetTextDrawHeight(enterPrompt);
	SetFontSize(20);
	const int hCtrl = GetTextDrawHeight(ctrlMove);
	SetFontSize(18);
	const int hFooter = GetTextDrawHeight(footer);

	const int sepBlockH = 6;
	const int enterTop = cy - hEnter / 2;
	const int taglineTop = enterTop - gapLg - hTagline;
	const int subtitleTop = taglineTop - gapMd - hSubtitle;
	const int sepLineY = subtitleTop - sepAfterLine - sepBlockH;
	const int logoTop = sepLineY - sepAfterLogo - hLogo;

	const int blockTop = logoTop - 20;
	const int blockBottom = enterTop + hEnter + gapLg + hCtrl + gapSm + hCtrl + gapMd + hFooter + 28;

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

	SetDrawBlendMode(DX_BLENDMODE_ALPHA, 48);
	DrawBox(cx - 460, blockTop, cx + 460, blockBottom, GetColor(8, 12, 32), TRUE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	DrawBox(cx - 460, blockTop, cx + 460, blockBottom, GetColor(0, 90, 130), FALSE);

	SetFontSize(84);
	DrawTextUtf8Centered(cx, logoTop, GetColor(255, 80, 120), logo);

	{
		const int lw = GetTextDrawWidth(logo);
		DrawLine(cx - lw / 2 - 24, sepLineY, cx + lw / 2 + 24, sepLineY, GetColor(0, 200, 255));
		DrawLine(cx - lw / 2 - 24, sepLineY + 3, cx + lw / 2 + 24, sepLineY + 3, GetColor(120, 40, 100));
	}

	SetFontSize(34);
	DrawTextUtf8Centered(cx, subtitleTop, GetColor(0, 230, 255), subtitle);

	SetFontSize(26);
	DrawTextUtf8Centered(cx, taglineTop, GetColor(220, 220, 220), tagline);

	if ((m_AnimTimer / 30) % 2 == 0)
	{
		SetFontSize(32);
		DrawTextUtf8Centered(cx, enterTop, GetColor(255, 255, 0), enterPrompt);
	}

	int y = enterTop + hEnter + gapLg;
	SetFontSize(20);
	DrawTextUtf8Centered(cx, y, GetColor(180, 200, 255), ctrlMove);
	y += hCtrl + gapSm;
	DrawTextUtf8Centered(cx, y, GetColor(180, 200, 255), ctrlShot);
	y += hCtrl + gapMd;

	SetFontSize(18);
	DrawTextUtf8Centered(cx, y, GetColor(140, 140, 160), footer);

	SetFontSize(20);
}
