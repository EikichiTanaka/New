#pragma once
#include "DxLib.h"

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

inline int DrawTextUtf8(int x, int y, unsigned int color, const char* text)
{
	return DrawFormatString(x, y, color, "%s", text);
}