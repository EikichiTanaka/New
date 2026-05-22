#pragma once
#include "GameConfig.h"
#include "Game/BulletManager.h"

// ============================================================
// Enemy.h
// 敵キャラクター単体のクラス定義
// 3種類の異なる移動パターンと攻撃弾幕を持ちます。
// ============================================================

enum class EnemyType
{
	Basic,
	Spinner,
	Tank,
	Shield,
	Splitter,
	Eraser,
	Sniper
};

class Enemy
{
public:
	// 敵の初期設定
	void Init(EnemyType type, float startX, float startZ, Difficulty diff);

	// 移動・攻撃パターンの更新
	void Update(float playerX, float playerZ, BulletManager& bullets);

	// 描画（3D立体キューブ＋頭上にHPゲージ）
	void Draw() const;

	// 被弾処理（ダメージを与え、HP0で撃破）
	void TakeDamage(int dmg);

	// 自機弾ヒット（同一敵への連続判定を間引く）
	bool TryApplyBulletDamage(int dmg);

	// ゲッター関数群
	float GetX() const { return m_X; }
	float GetY() const { return m_Y; }
	float GetZ() const { return m_Z; }
	float GetRadius() const { return m_Radius; }
	bool IsActive() const { return m_Active; }
	int GetHp() const { return m_Hp; }
	int GetMaxHp() const { return m_MaxHp; }
	EnemyType GetType() const { return m_Type; }

	// 強制非アクティブ化
	void Deactivate() { m_Active = false; }

private:
	float m_X, m_Y, m_Z;  // 3D座標
	float m_VX, m_VZ;     // 速度
	int m_Hp;             // 現在のHP
	int m_MaxHp;          // 最大HP（ゲージ表示用）
	EnemyType m_Type;     // 敵のタイプ
	float m_Radius;       // 衝突判定の半径
	bool m_Active;        // 生存フラグ

	int m_Timer;          // 行動パターン用のフレームタイマー
	int m_FireRate;       // 弾の発射間隔フレーム
	float m_BulletSpeed;  // 難易度に基づいた弾速
	int m_HitCooldown;    // 自機弾連続ヒット抑制
};
