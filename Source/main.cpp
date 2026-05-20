// =============================================================================
// main.cpp
// エントリポイント WinMain とメインループ
// =============================================================================

#include "DxLib.h"
#include "GameConfig.h"
#include "Manager/SceneManager.h"
#include "Common/KeyHelper.h"
#include "Common/SoundSynth.h"

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	(void)hInstance;
	(void)hPrevInstance;
	(void)lpCmdLine;
	(void)nCmdShow;

	if (DxLib_Init() == -1)
	{
		return -1;
	}

	SetUseCharCodeFormat(DX_CHARCODEFORMAT_SHIFTJIS);

	SetWindowSizeExtendRate(1.0);
	ChangeWindowMode(TRUE);
	SetGraphMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32);
	SetMainWindowText(WINDOW_TITLE);

	SoundSynth::Init();
	SceneManager::GetInstance().Init();

	while (ProcessMessage() == 0)
	{
		// フレームの最初にキー状態を更新
		KeyHelper::Update();

		if (CheckHitKey(KEY_INPUT_ESCAPE) != 0)
		{
			break;
		}

		SetDrawScreen(DX_SCREEN_BACK);
		ClearDrawScreen();

		SceneManager::GetInstance().Update();
		SceneManager::GetInstance().Draw();

		ScreenFlip();
	}

	SceneManager::GetInstance().Final();
	SoundSynth::Final();
	DxLib_End();

	return 0;
}
