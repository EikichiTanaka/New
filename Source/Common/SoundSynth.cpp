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

	static bool s_Initialized = false;
	static int s_SndPaddleBounce = 0;
	static int s_SndPlayerShoot = 0;
	static int s_SndExplosion = 0;
	static int s_SndGraze = 0;
	static int s_SndBomb = 0;
	static int s_SndFeverReady = 0;
	static int s_SndFeverStart = 0;

	int CreateSynthSoundMem(int startFreq, int endFreq, int durationMs, int waveType, int volume)
	{
		int sampleRate = 44100;
		int numSamples = (sampleRate * durationMs) / 1000;
		int dataSize = numSamples * 2;
		int fileSize = 44 + dataSize;

		std::vector<char> buffer(fileSize);

		memcpy(&buffer[0], "RIFF", 4);
		int chunk32 = fileSize - 8;
		memcpy(&buffer[4], &chunk32, 4);
		memcpy(&buffer[8], "WAVE", 4);

		memcpy(&buffer[12], "fmt ", 4);
		int subchunk1Size = 16;
		memcpy(&buffer[16], &subchunk1Size, 4);
		short audioFormat = 1;
		memcpy(&buffer[20], &audioFormat, 2);
		short numChannels = 1;
		memcpy(&buffer[22], &numChannels, 2);
		memcpy(&buffer[24], &sampleRate, 4);
		int byteRate = sampleRate * 2;
		memcpy(&buffer[28], &byteRate, 4);
		short blockAlign = 2;
		memcpy(&buffer[32], &blockAlign, 2);
		short bitsPerSample = 16;
		memcpy(&buffer[34], &bitsPerSample, 2);

		memcpy(&buffer[36], "data", 4);
		memcpy(&buffer[40], &dataSize, 4);

		short* pData = (short*)&buffer[44];
		float T = (float)durationMs / 1000.0f;

		for (int i = 0; i < numSamples; i++)
		{
			float t = (float)i / sampleRate;
			float decay = 1.0f - ((float)i / numSamples);

			float phase = 2.0f * 3.14159265f * (startFreq * t + (endFreq - startFreq) * t * t / (2.0f * T));

			float amp = 0.0f;
			if (waveType == 0)
				amp = sinf(phase);
			else if (waveType == 1)
				amp = (sinf(phase) > 0.0f) ? 0.6f : -0.6f;
			else if (waveType == 2)
			{
				float randNoise = (float)(rand() % 200 - 100) / 100.0f;
				amp = sinf(phase) * 0.35f + randNoise * 0.65f;
			}

			short val = (short)(16000 * amp * decay * (volume / 255.f));
			pData[i] = val;
		}

		return LoadSoundMemByMemImage(buffer.data(), fileSize);
	}

	void Init()
	{
		if (s_Initialized)
			return;

		s_SndPaddleBounce = CreateSynthSoundMem(440, 440, 60, 0, 160);
		s_SndPlayerShoot = CreateSynthSoundMem(800, 1600, 80, 0, 120);
		s_SndExplosion = CreateSynthSoundMem(150, 20, 320, 2, 255);
		s_SndGraze = CreateSynthSoundMem(2000, 3600, 40, 0, 165);
		s_SndBomb = CreateSynthSoundMem(120, 1400, 650, 1, 240);
		s_SndFeverReady = CreateSynthSoundMem(600, 900, 120, 0, 130);
		s_SndFeverStart = CreateSynthSoundMem(400, 1800, 280, 0, 200);

		s_Initialized = true;
		GameOptionsApplyVolumes();
	}

	void PlayPaddleBounce()
	{
		PlaySe(s_SndPaddleBounce);
	}

	void PlayPlayerShoot()
	{
		PlaySe(s_SndPlayerShoot);
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

	void Final()
	{
		if (!s_Initialized)
			return;

		if (s_SndPaddleBounce != 0) DeleteSoundMem(s_SndPaddleBounce);
		if (s_SndPlayerShoot != 0) DeleteSoundMem(s_SndPlayerShoot);
		if (s_SndExplosion != 0) DeleteSoundMem(s_SndExplosion);
		if (s_SndGraze != 0) DeleteSoundMem(s_SndGraze);
		if (s_SndBomb != 0) DeleteSoundMem(s_SndBomb);
		if (s_SndFeverReady != 0) DeleteSoundMem(s_SndFeverReady);
		if (s_SndFeverStart != 0) DeleteSoundMem(s_SndFeverStart);

		s_SndPaddleBounce = 0;
		s_SndPlayerShoot = 0;
		s_SndExplosion = 0;
		s_SndGraze = 0;
		s_SndBomb = 0;
		s_SndFeverReady = 0;
		s_SndFeverStart = 0;
		s_Initialized = false;
	}
}
