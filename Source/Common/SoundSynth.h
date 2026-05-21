#pragma once

// ============================================================
// SoundSynth.h
// DXライブラリのメモリサウンド作成APIを使用したリアルタイム効果音ジェネレータ
// 外部音声ファイルが一切不要で、軽量かつレトロなサイファイ音を自動生成します。
// ============================================================

namespace SoundSynth
{
	// 初期化（サウンドアセットのメモリ合成・読込）
	void Init();

	// 終了処理（作成したサウンドバッファの破棄）
	void Final();

	// --- 互換性維持用の旧ブロック崩し音 ---
	void PlayBlockBreak(int combo);
	void PlayPaddleBounce();
	void PlaySmash();

	// --- 3D弾幕シューティング用のプレミアムサイファイ効果音 ---
	void PlayPlayerShoot();  // 自機ショット音（高音レーザー）
	void PlayEnemyShoot();   // 敵ショット音（短いくぐもった音）
	void PlayExplosion();    // 撃破爆発音（ドスッと響くノイズ風爆発）
	void PlayGraze();        // グレイズ音（キィンという高音チャープ）
	void PlayBomb();
	void PlayFeverReady();
	void PlayFeverStart();
}
