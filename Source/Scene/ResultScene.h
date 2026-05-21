#pragma once

// ============================================================
// ResultScene.h
// リザルト画面シーン
//
// 【役割】
// ゲーム終了後のスコアやプレイ結果を表示する。
// ENTER キーでタイトル画面に戻る。
// ============================================================

#include "Scene/SceneBase.h"

class ResultScene : public SceneBase
{
public:
	void Init() override;
	SceneType Update() override;
	void Draw() override;

private:
	void FinalizeAchievements();

	int m_AnimTimer;
};
