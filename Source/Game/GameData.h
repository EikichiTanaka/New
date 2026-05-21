#pragma once
#include "GameConfig.h"

// ============================================================
// GameData.h
// シーン間でデータを共有するためのグローバル構造体
//
// 【使い方】
// - DifficultySelectScene で g_GameData.difficulty を設定
// - GameScene で score, lives などを更新
// - ResultScene で結果を表示
// ============================================================

struct GameData
{
	// --- 設定 ---
	Difficulty difficulty = Difficulty::Normal;

	// --- プレイ中のデータ ---
	int score          = 0;
	int lives          = START_LIVES;
	int maxCombo       = 0;
	int enemiesDefeated = 0;
	int grazeCount     = 0;     // グレイズ回数
	int currentWave    = 0;
	int playTimeSec    = 0;

	// --- 終了状態 ---
	bool isClear    = false;
	bool isGameOver = false;

	// --- リセット ---
	void Reset()
	{
		score = 0;
		lives = START_LIVES;
		maxCombo = 0;
		enemiesDefeated = 0;
		grazeCount = 0;
		currentWave = 0;
		playTimeSec = 0;
		isClear = false;
		isGameOver = false;
	}
};

// グローバルインスタンス（extern宣言）
// 実体は GameData.cpp で定義
extern GameData g_GameData;
