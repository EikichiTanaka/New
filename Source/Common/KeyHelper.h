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

	// ボール発射・決定用（Enter / テンキーEnter / Space）
	static bool IsLaunchTrigger();
};
