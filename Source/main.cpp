// ============================================================
// main.cpp
// ゲームのエントリーポイント（プログラム起点）
//
// 【主な責務】
// 1. DxLib の初期化（ウィンドウ生成・文字コード設定など）
// 2. SceneManager の生成（初期シーン作成）
// 3. メインループ（毎フレーム Update → Draw を回す）
// 4. DxLib の終了処理
// ============================================================

#include "DxLib.h"
#include "GameConfig.h"
#include "Manager/SceneManager.h"
#include "Common/SaveData.h"
#include "Common/KeyHelper.h"
#include "Common/GameOptions.h"
#include "Common/BgmPlayer.h"
#include "Common/GameScreen.h"
#include "Common/ResourceManager.h"

// ============================================================
// WinMain: Windows アプリケーションのエントリーポイント
// ============================================================
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	// ソースコードの文字コードを UTF-8 として扱うよう DxLib に通知
	// （これにより日本語文字列がそのまま正しく描画される）
	SetUseCharCodeFormat(DX_CHARCODEFORMAT_UTF8);

	SetMainWindowText(WINDOW_TITLE);

	// DxLib 初期化
	if (DxLib_Init() == -1) return -1;

	GameOptionsLoad();
	GameOptionsApplyGraphics();
	GameOptionsApplyVolumes();
	BgmPlayer::Init();

	// 日本語表示用フォント（UTF-8 文字列）
	SetFontSize(20);
	ChangeFont("Meiryo");

	SaveDataLoad();

	SceneManager sceneManager;
	sceneManager.Init();

	// メインゲームループ
	while (ProcessMessage() == 0)
	{
		KeyHelper::Update();

		ClearDrawScreen();

		if (!sceneManager.Update())
		{
			break;
		}

		sceneManager.Draw();
		ScreenFlip();
	}

	ResourceManager::Final();
	BgmPlayer::Final();
	GameOptionsSave();
	DxLib_End();
	return 0;
}