#include "Common/SoundSynth.h"
#include "Common/GameOptions.h"
#include "DxLib.h"
#include <vector>
#include <cmath>
#include <cstdlib>

namespace SoundSynth
{
	static void PlaySe(int handle)
	{
		if (handle == 0) return;
		PlaySoundMem(handle, DX_PLAYTYPE_BACK);
		ChangeVolumeSoundMem(GameOptionsGetSeVolume255(), handle);
	}

	// クラシック音（互換用）
	static int s_SndBlockBreak[8] = {};
	static int s_SndPaddleBounce = 0;
	static int s_SndSmash = 0;

	// 新規プレミアム効果音
	static int s_SndPlayerShoot = 0;
	static int s_SndEnemyShoot = 0;
	static int s_SndExplosion = 0;
	static int s_SndGraze = 0;
	static int s_SndBomb = 0;
	static int s_SndFeverReady = 0;
	static int s_SndFeverStart = 0;

	// --- CreateSynthSoundMem: 周波数スイープ（チャープ）対応のメモリサウンド生成 ---
	// waveType: 0=正弦波(Sine), 1=矩形波(Square), 2=ノイズ混合波(Noise Mix)
	int CreateSynthSoundMem(int startFreq, int endFreq, int durationMs, int waveType, int volume)
	{
		int sampleRate = 44100;
		int numSamples = (sampleRate * durationMs) / 1000;
		int dataSize = numSamples * 2; // 16bit Mono (1サンプル2バイト)
		int fileSize = 44 + dataSize;  // WAVEヘッダ(44バイト) + データ本体

		std::vector<char> buffer(fileSize);

		// 1. RIFF WAVEヘッダの構築
		memcpy(&buffer[0], "RIFF", 4);
		int chunk32 = fileSize - 8;
		memcpy(&buffer[4], &chunk32, 4);
		memcpy(&buffer[8], "WAVE", 4);

		// Sub-chunk 1 (fmt )
		memcpy(&buffer[12], "fmt ", 4);
		int subchunk1Size = 16;
		memcpy(&buffer[16], &subchunk1Size, 4);
		short audioFormat = 1; // PCM
		memcpy(&buffer[20], &audioFormat, 2);
		short numChannels = 1; // Mono
		memcpy(&buffer[22], &numChannels, 2);
		memcpy(&buffer[24], &sampleRate, 4);
		int byteRate = sampleRate * 2;
		memcpy(&buffer[28], &byteRate, 4);
		short blockAlign = 2;
		memcpy(&buffer[32], &blockAlign, 2);
		short bitsPerSample = 16;
		memcpy(&buffer[34], &bitsPerSample, 2);

		// Sub-chunk 2 (data)
		memcpy(&buffer[36], "data", 4);
		memcpy(&buffer[40], &dataSize, 4);

		// 2. 音声波形の合成
		short* pData = (short*)&buffer[44];
		float T = (float)durationMs / 1000.0f; // 総時間（秒）

		for (int i = 0; i < numSamples; i++)
		{
			float t = (float)i / sampleRate;       // 現在時刻（秒）
			float decay = 1.0f - ((float)i / numSamples); // 線形音量減衰（フェードアウト）
			
			// 数学的に完璧な線形周波数スイープ（Chirp）の位相計算
			float phase = 2.0f * 3.14159265f * (startFreq * t + (endFreq - startFreq) * t * t / (2.0f * T));

			float amp = 0.0f;
			if (waveType == 0)
			{
				// 正弦波（滑らかでクリアなレーザーチャープ）
				amp = sinf(phase);
			}
			else if (waveType == 1)
			{
				// 矩形波（重厚でピコピコ感のあるファミコン風レーザー）
				amp = (sinf(phase) > 0.0f) ? 0.6f : -0.6f;
			}
			else if (waveType == 2)
			{
				// ノイズブレンド波（ホワイトノイズを混ぜた爆発表現）
				float randNoise = (float)(rand() % 200 - 100) / 100.0f; // -1.0f ～ 1.0f
				amp = sinf(phase) * 0.35f + randNoise * 0.65f;
			}

			// 音量スケールを考慮して16bit符号付き整数にスケール
			short val = (short)(16000 * amp * decay * (volume / 255.f));
			pData[i] = val;
		}

		// DXライブラリのメモリ読み込みAPIでサウンドハンドルを生成
		return LoadSoundMemByMemImage(buffer.data(), fileSize);
	}

	// --- Init: サウンドリソースのリアルタイム生成 ---
	void Init()
	{
		// 1. クラシック音（ブロック破壊用 8音音階）
		int freqs[8] = { 523, 587, 659, 784, 880, 1046, 1174, 1318 };
		for (int i = 0; i < 8; i++)
		{
			s_SndBlockBreak[i] = CreateSynthSoundMem(freqs[i], freqs[i], 180, 0, 150);
		}
		s_SndPaddleBounce = CreateSynthSoundMem(440, 440, 60, 0, 160);
		s_SndSmash = CreateSynthSoundMem(90, 45, 400, 1, 255); // 重低音スイープ

		// 2. プレミアム弾幕シューティング用効果音
		s_SndPlayerShoot = CreateSynthSoundMem(800, 1600, 80, 0, 120);     // 800Hzから1600Hzへの高速上昇サインチャープ（爽快レーザー）
		s_SndEnemyShoot  = CreateSynthSoundMem(500, 200, 65, 0, 80);       // 500Hzから200Hzへの下降サインチャープ（敵弾）
		s_SndExplosion   = CreateSynthSoundMem(150, 20, 320, 2, 255);      // 150Hzから20Hzへのノイズ混合スイープ（重厚な爆発音）
		s_SndGraze       = CreateSynthSoundMem(2000, 3600, 40, 0, 165);    // 超高速・超高音の金属的チャープ（グレイズ音）
		s_SndBomb        = CreateSynthSoundMem(120, 1400, 650, 1, 240);
		s_SndFeverReady  = CreateSynthSoundMem(600, 900, 120, 0, 130);
		s_SndFeverStart  = CreateSynthSoundMem(400, 1800, 280, 0, 200);
		GameOptionsApplyVolumes();
	}

	// --- 互換用関数群 ---
	void PlayBlockBreak(int combo)
	{
		int idx = (combo - 1) % 8;
		if (idx < 0) idx = 0;
		PlaySe(s_SndBlockBreak[idx]);
	}

	void PlayPaddleBounce()
	{
		PlaySe(s_SndPaddleBounce);
	}

	void PlaySmash()
	{
		PlaySe(s_SndSmash);
	}

	// --- 新規シューティング効果音再生関数群 ---
	void PlayPlayerShoot()
	{
		PlaySe(s_SndPlayerShoot);
	}

	void PlayEnemyShoot()
	{
		PlaySe(s_SndEnemyShoot);
	}

	void PlayExplosion()
	{
		PlaySe(s_SndExplosion);
	}

	void PlayGraze()
	{
		PlaySe(s_SndGraze);
	}

	void PlayBomb()
	{
		PlaySe(s_SndBomb);
	}

	void PlayFeverReady()
	{
		PlaySe(s_SndFeverReady);
	}

	void PlayFeverStart()
	{
		PlaySe(s_SndFeverStart);
	}

	// --- Final: メモリ破棄 ---
	void Final()
	{
		for (int i = 0; i < 8; i++)
		{
			if (s_SndBlockBreak[i] != 0) DeleteSoundMem(s_SndBlockBreak[i]);
		}
		if (s_SndPaddleBounce != 0) DeleteSoundMem(s_SndPaddleBounce);
		if (s_SndSmash != 0) DeleteSoundMem(s_SndSmash);

		if (s_SndPlayerShoot != 0) DeleteSoundMem(s_SndPlayerShoot);
		if (s_SndEnemyShoot != 0) DeleteSoundMem(s_SndEnemyShoot);
		if (s_SndExplosion != 0) DeleteSoundMem(s_SndExplosion);
		if (s_SndGraze != 0) DeleteSoundMem(s_SndGraze);
		if (s_SndBomb != 0) DeleteSoundMem(s_SndBomb);
		if (s_SndFeverReady != 0) DeleteSoundMem(s_SndFeverReady);
		if (s_SndFeverStart != 0) DeleteSoundMem(s_SndFeverStart);
	}
}
