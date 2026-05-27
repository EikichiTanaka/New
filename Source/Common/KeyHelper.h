#pragma once

// 1フレームだけ押されたキーを判定
class KeyHelper
{
public:
	// フレームの最初に1回だけ呼ぶ（main.cpp）
	static void Update();

	// シーン切替直後に呼ぶ（前の画面で押していたキーを無効化）
	static void SyncCurrentKeys();

	static bool IsTrigger(int dxKey);
	static bool IsRepeat(int dxKey, int firstDelayFrames = 14, int intervalFrames = 4);
	static bool IsMenuMoveTrigger(int dxKey);

	// ボール発射・決定用（Enter / テンキーEnter / Space）
	static bool IsLaunchTrigger();

	// メニュー決定（Enter / テンキーEnter）1回押しのみ
	static bool IsConfirmTrigger();

	// メニュー戻る・終了（Esc）1回押しのみ
	static bool IsCancelTrigger();
};
