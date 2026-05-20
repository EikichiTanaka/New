#include "Common/KeyHelper.h"
#include "DxLib.h"

static int s_Prev[256] = {};
static int s_Current[256] = {};

void KeyHelper::Update()
{
	for (int i = 0; i < 256; i++)
	{
		s_Prev[i] = s_Current[i];
		s_Current[i] = CheckHitKey(i);
	}
}

void KeyHelper::SyncCurrentKeys()
{
	for (int i = 0; i < 256; i++)
	{
		s_Current[i] = CheckHitKey(i);
		s_Prev[i] = s_Current[i];
	}
}

bool KeyHelper::IsTrigger(int key)
{
	if (key < 0 || key >= 256) return false;
	return s_Current[key] != 0 && s_Prev[key] == 0;
}

bool KeyHelper::IsLaunchTrigger()
{
	if (IsTrigger(KEY_INPUT_RETURN)) return true;
	if (IsTrigger(KEY_INPUT_NUMPADENTER)) return true;
	if (IsTrigger(KEY_INPUT_SPACE)) return true;
	return false;
}
