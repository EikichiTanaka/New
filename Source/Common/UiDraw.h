#pragma once
#include "DxLib.h"

inline int DrawTextUtf8(int x, int y, unsigned int color, const char* text)
{
	return DrawFormatString(x, y, color, "%s", text);
}