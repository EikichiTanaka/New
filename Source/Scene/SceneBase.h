#pragma once

// ============================================================
// SceneBase.h
// すべてのシーンが継承する基底クラス（インターフェース）
// ============================================================

// シーン種別
enum class SceneType
{
	None,              // 遷移しない（現在のシーンを継続）
	Title,             // タイトル画面
	Options,           // オプション画面
	DifficultySelect,  // 難易度選択画面
	Game,              // ゲーム本編
	Tutorial,          // 操作チュートリアル（本編とは別シーン）
	Result,            // リザルト画面
	Exit               // ゲーム終了
};

class SceneBase
{
public:
	virtual ~SceneBase() = default;

	// 初期化処理
	virtual void Init() = 0;

	// 毎フレームの更新処理
	// 戻り値: 次に遷移するシーン（遷移しない場合は SceneType::None）
	virtual SceneType Update() = 0;

	// 毎フレームの描画処理
	virtual void Draw() = 0;
};