#include "Common/KeyHelper.h"
#include "DxLib.h"

static int s_Prev[256] = {};
static int s_Current[256] = {};
static int s_HoldFrames[256] = {};

void KeyHelper::Update()
{
	for (int i = 0; i < 256; i++)
	{
		s_Prev[i] = s_Current[i];
		s_Current[i] = CheckHitKey(i);
		if (s_Current[i] != 0)
			s_HoldFrames[i]++;
		else
			s_HoldFrames[i] = 0;
	}
}

void KeyHelper::SyncCurrentKeys()
{
	for (int i = 0; i < 256; i++)
	{
		s_Current[i] = CheckHitKey(i);
		s_Prev[i] = s_Current[i];
		s_HoldFrames[i] = s_Current[i] != 0 ? 1 : 0;
	}
}

bool KeyHelper::IsTrigger(int key)
{
	if (key < 0 || key >= 256) return false;
	return s_Current[key] != 0 && s_Prev[key] == 0;
}

bool KeyHelper::IsRepeat(int key, int firstDelayFrames, int intervalFrames)
{
	if (key < 0 || key >= 256) return false;
	if (s_Current[key] == 0) return false;
	if (firstDelayFrames < 1) firstDelayFrames = 1;
	if (intervalFrames < 1) intervalFrames = 1;

	const int held = s_HoldFrames[key];
	if (held <= firstDelayFrames) return false;
	return ((held - firstDelayFrames) % intervalFrames) == 0;
}

bool KeyHelper::IsMenuMoveTrigger(int key)
{
	// メニュー操作は体感重視で早めにリピート開始
	return IsTrigger(key) || IsRepeat(key, 10, 2);
}

bool KeyHelper::IsLaunchTrigger()
{
	if (IsTrigger(KEY_INPUT_RETURN)) return true;
	if (IsTrigger(KEY_INPUT_NUMPADENTER)) return true;
	if (IsTrigger(KEY_INPUT_SPACE)) return true;
	return false;
}

bool KeyHelper::IsConfirmTrigger()
{
	return IsTrigger(KEY_INPUT_RETURN) || IsTrigger(KEY_INPUT_NUMPADENTER);
}

bool KeyHelper::IsCancelTrigger()
{
	return IsTrigger(KEY_INPUT_ESCAPE);
}
