#pragma once
#include "GameConfig.h"

// ============================================================
// Effect.h
// 爆発エフェクト、グレイズスパーク、ボム衝撃波、
// および高速スクロールする3D背景スターフィールドを管理するクラス
// ============================================================

struct Particle
{
	float x, y, z;       // 3D座標
	float vx, vy, vz;    // 移動速度
	int life;            // 残り寿命フレーム数
	int maxLife;         // 最大寿命（アルファフェードアウト用）
	unsigned int color;  // カラー
	float radius;        // パーティクルの描画半径
	bool active;         // アクティブフラグ
};

struct Star
{
	float x, y, z;       // 背景流星の3D座標
	float speed;         // スクロール速度
	unsigned int color;  // カラー
};

class Effect
{
public:
	// 初期化（流星群、パーティクルプールのセットアップ）
	void Init();

	// エフェクト群、ボム衝撃波、背景流星の更新
	void Update();

	// エフェクトと背景流星の3D描画
	void Draw() const;

	// 爆発エフェクトを発生（敵撃破時など）
	void AddExplosion(float x, float y, float z, unsigned int color, int count = 0);

	// グレイズ発生時の火花エフェクト
	void AddGrazeSpark(float x, float y, float z);

	// アイテム回収時のキラキラエフェクト
	void AddItemSparkle(float x, float y, float z, unsigned int color);

	// ボムの巨大膨張衝撃波を発動
	void TriggerBombShockwave(float x, float y, float z);

	// ボム衝撃波の現在半径ゲッター
	float GetBombShockwaveRadius() const { return m_BombActive ? m_BombRadius : 0.0f; }
	bool IsBombActive() const { return m_BombActive; }

private:
	// パーティクル配列プール
	Particle m_Particles[PARTICLE_MAX];

	// 背景の3Dスクロール流星群
	static constexpr int STAR_MAX = 32;
	Star m_Stars[STAR_MAX];

	// ボム衝撃波のパラメータ
	bool m_BombActive;
	float m_BombX, m_BombY, m_BombZ;
	float m_BombRadius;
	int m_BombTimer;
};
