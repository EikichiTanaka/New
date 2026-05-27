#pragma once
#include "DxLib.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

// 弾幕描画：シーンとの奥行き判定はするが、弾同士では Z 書き込みしない（加算合成が潰れない）
inline void BeginBulletDraw3D()
{
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(FALSE);
}

// 3D描画後に2D UI を重ねる前に呼ぶ（Zバッファ有効のままだと HUD が描画されない）
inline void BeginScreenSpaceDraw()
{
	SetUseZBuffer3D(FALSE);
	SetWriteZBuffer3D(FALSE);
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);
	SetUseLighting(FALSE);
	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
}

// 現在の SetFontSize に合わせた描画幅（中央揃え用）
inline int GetTextDrawWidth(const char* text)
{
	if (text == nullptr || text[0] == '\0')
		return 0;
	return GetDrawStringWidth(text, (int)strlen(text), FALSE);
}

inline int GetTextDrawHeight(const char* text)
{
	if (text == nullptr || text[0] == '\0')
		return 0;
	int sx = 0;
	int sy = 0;
	int lineCount = 0;
	GetDrawStringSize(&sx, &sy, &lineCount, text, (int)strlen(text), FALSE);
	return sy;
}

inline int DrawTextUtf8(int x, int y, unsigned int color, const char* text)
{
	return DrawString(x, y, text, color);
}

inline int DrawTextUtf8Centered(int centerX, int y, unsigned int color, const char* text)
{
	const int w = GetTextDrawWidth(text);
	return DrawString(centerX - w / 2, y, text, color);
}

inline int DrawFormatStringCentered(int centerX, int y, unsigned int color, const char* fmt, ...)
{
	char buf[512];
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	buf[sizeof(buf) - 1] = '\0';
	return DrawTextUtf8Centered(centerX, y, color, buf);
}
