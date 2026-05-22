#pragma once

// ============================================================
// SceneManager.h
// シーン（画面）の生成と遷移を管理するクラス
// ============================================================

#include "Scene/SceneBase.h"

class SceneManager
{
public:
	SceneManager();
	~SceneManager();

	// 初期シーンを生成して開始する
	void Init();

	// 現在のシーンを更新し、必要に応じて遷移する
	// 戻り値: ゲーム続行ならば true、終了するなら false
	bool Update();

	// 現在のシーンを描画する
	void Draw();

private:
	// 指定したシーンに切り替える
	void ChangeScene(SceneType nextScene);

	// 現在アクティブなシーンへのポインタ
	SceneBase* m_pCurrentScene;
};