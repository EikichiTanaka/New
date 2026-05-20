#include "Common/SoundSynth.h"
#include "DxLib.h"
#include <vector>
#include <math.h>

namespace SoundSynth
{
	static int s_SndBlockBreak[8] = {};
	static int s_SndPaddleBounce = 0;
	static int s_SndSmash = 0;

	int CreateSynthSoundMem(int freq, int durationMs, int waveType, int volume)
	{
		int sampleRate = 44100;
		int numSamples = (sampleRate * durationMs) / 1000;
		int dataSize = numSamples * 2; // 16bit Mono
		int fileSize = 44 + dataSize;

		std::vector<char> buffer(fileSize);

		// RIFF Header
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

		short* pData = (short*)&buffer[44];
		for (int i = 0; i < numSamples; i++)
		{
			float t = (float)i / sampleRate;
			float decay = 1.0f - ((float)i / numSamples);
			
			// Slight frequency sweep for juicy feel
			float currentFreq = (float)freq;
			if (waveType == 1) // Smash sweep down
			{
				currentFreq = freq - (freq * 0.5f * ((float)i / numSamples));
			}

			float amp = sinf(2.0f * 3.14159265f * currentFreq * t);
			short val = 0;

			if (waveType == 0) // Sine wave (smooth block breaks)
			{
				val = (short)(15000 * amp);
			}
			else if (waveType == 1) // Square wave (thick heavy smash)
			{
				val = (amp > 0.0f) ? 8000 : -8000;
			}

			pData[i] = (short)(val * decay * (volume / 255.f));
		}

		return LoadSoundMemByMemImage(buffer.data(), fileSize);
	}

	void Init()
	{
		// 8 pentatonic notes: C5, D5, E5, G5, A5, C6, D6, E6
		int freqs[8] = { 523, 587, 659, 784, 880, 1046, 1174, 1318 };
		for (int i = 0; i < 8; i++)
		{
			s_SndBlockBreak[i] = CreateSynthSoundMem(freqs[i], 180, 0, 150);
		}
		s_SndPaddleBounce = CreateSynthSoundMem(440, 60, 0, 160);
		s_SndSmash = CreateSynthSoundMem(90, 400, 1, 255); // Heavy 90Hz square sweep
	}

	void PlayBlockBreak(int combo)
	{
		int idx = (combo - 1) % 8;
		if (idx < 0) idx = 0;
		PlaySoundMem(s_SndBlockBreak[idx], DX_PLAYTYPE_BACK);
	}

	void PlayPaddleBounce()
	{
		PlaySoundMem(s_SndPaddleBounce, DX_PLAYTYPE_BACK);
	}

	void PlaySmash()
	{
		PlaySoundMem(s_SndSmash, DX_PLAYTYPE_BACK);
	}

	void Final()
	{
		for (int i = 0; i < 8; i++)
		{
			if (s_SndBlockBreak[i] != 0) DeleteSoundMem(s_SndBlockBreak[i]);
		}
		if (s_SndPaddleBounce != 0) DeleteSoundMem(s_SndPaddleBounce);
		if (s_SndSmash != 0) DeleteSoundMem(s_SndSmash);
	}
}

//#include "Common/SoundSynth.h"
//#include "DxLib.h"
//namespace SoundSynth
//{
//	static int s_SndBlockBreak[8] = {};
//	static int s_SndPaddleBounce = 0;
//	static int s_SndSmash = 0;
//	void Init()
//	{
//		// === 外部ファイルを読み込むように書き換える ===
//
//		// 1. パドル反射音を読み込む
//		s_SndPaddleBounce = LoadSoundMem("Assets/se_bounce.wav");
//
//		// 2. スマッシュ音を読み込む
//		s_SndSmash = LoadSoundMem("Assets/se_smash.wav");
//
//		// 3. ブロック破壊音（8音階）を順に読み込む
//		s_SndBlockBreak[0] = LoadSoundMem("Assets/se_break1.wav");
//		s_SndBlockBreak[1] = LoadSoundMem("Assets/se_break2.wav");
//		s_SndBlockBreak[2] = LoadSoundMem("Assets/se_break3.wav");
//		s_SndBlockBreak[3] = LoadSoundMem("Assets/se_break4.wav");
//		s_SndBlockBreak[4] = LoadSoundMem("Assets/se_break5.wav");
//		s_SndBlockBreak[5] = LoadSoundMem("Assets/se_break6.wav");
//		s_SndBlockBreak[6] = LoadSoundMem("Assets/se_break7.wav");
//		s_SndBlockBreak[7] = LoadSoundMem("Assets/se_break8.wav");
//	}
//	void PlayBlockBreak(int combo)
//	{
//		int idx = (combo - 1) % 8;
//		if (idx < 0) idx = 0;
//		// 既に再生中なら最初から再生し直す（再生が被っても途切れないようにする）
//		PlaySoundMem(s_SndBlockBreak[idx], DX_PLAYTYPE_BACK, TRUE);
//	}
//	void PlayPaddleBounce()
//	{
//		PlaySoundMem(s_SndPaddleBounce, DX_PLAYTYPE_BACK, TRUE);
//	}
//	void PlaySmash()
//	{
//		PlaySoundMem(s_SndSmash, DX_PLAYTYPE_BACK, TRUE);
//	}
//	void Final()
//	{
//		// メモリ解放処理（これはそのままでOK！）
//		for (int i = 0; i < 8; i++)
//		{
//			if (s_SndBlockBreak[i] != 0) DeleteSoundMem(s_SndBlockBreak[i]);
//		}
//		if (s_SndPaddleBounce != 0) DeleteSoundMem(s_SndPaddleBounce);
//		if (s_SndSmash != 0) DeleteSoundMem(s_SndSmash);
//	}
//}
