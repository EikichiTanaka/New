#include "Common/GameScreen.h"
#include "Common/ResourceManager.h"
#include "Game/TouhouTheme.h"
#include "GameConfig.h"
#include "DxLib.h"

int g_ScreenWidth = DESIGN_SCREEN_WIDTH;
int g_ScreenHeight = DESIGN_SCREEN_HEIGHT;

void GameScreenSyncSize()
{
	int w = 0, h = 0;
	if (GetDrawScreenSize(&w, &h) == 0 && w > 0 && h > 0)
	{
		g_ScreenWidth = w;
		g_ScreenHeight = h;
	}
}

void GameReloadGraphicsResources()
{
	SetDrawScreen(DX_SCREEN_BACK);
	GameScreenSyncSize();

	// SetGraphMode 後は Z バッファ等が初期化されるため 3D 用に再設定
	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(TRUE);
	SetUseZBufferFlag(FALSE);
	SetWriteZBufferFlag(FALSE);

	// 画面モード変更で MV1 / MakeScreen ハンドルが無効化される
	ResourceManager::Init();
	if (UseTouhouTheme())
		TouhouTheme::Init();
}
