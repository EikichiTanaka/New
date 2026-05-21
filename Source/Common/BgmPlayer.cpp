#include "Common/BgmPlayer.h"
#include "Common/GameOptions.h"
#include "DxLib.h"
#include <vector>
#include <cmath>
#include <cstring>

namespace BgmPlayer
{
	static int s_Music[5] = {};
	static BgmTrack s_Current = BgmTrack::None;
	static int s_Loaded = 0;

	static int CreateLoopBgm(int bpm, int rootHz, int bars, int waveType, float bassMix)
	{
		const int sampleRate = 44100;
		const int beatsPerBar = 4;
		const int totalBeats = bars * beatsPerBar;
		const float beatSec = 60.0f / (float)bpm;
		const int numSamples = (int)(sampleRate * beatSec * totalBeats);
		const int dataSize = numSamples * 2;
		const int fileSize = 44 + dataSize;

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

		const int scale[] = { 0, 2, 4, 7, 9 };
		const int scaleLen = 5;

		short* pData = (short*)&buffer[44];
		float phaseBass = 0.0f;
		float phaseMel = 0.0f;
		float melFreq = (float)rootHz;

		for (int i = 0; i < numSamples; i++)
		{
			float t = (float)i / sampleRate;
			int beat = (int)(t / beatSec);
			if (beat >= totalBeats) beat = totalBeats - 1;
			int step = (beat / 2) % scaleLen;
			int targetHz = rootHz * (int)powf(2.0f, scale[step] / 12.0f);
			melFreq += (targetHz - melFreq) * 0.02f;

			float bassHz = (float)rootHz * 0.5f;
			phaseBass += 2.0f * 3.14159265f * bassHz / sampleRate;
			phaseMel += 2.0f * 3.14159265f * melFreq / sampleRate;

			float amp = sinf(phaseMel) * 0.35f + sinf(phaseBass) * bassMix;
			if (waveType == 1)
				amp = (sinf(phaseMel) > 0 ? 0.28f : -0.28f) + sinf(phaseBass) * bassMix * 0.8f;

			short val = (short)(12000.0f * amp);
			pData[i] = val;
		}

		return LoadSoundMemByMemImage(buffer.data(), fileSize);
	}

	void Init()
	{
		if (s_Loaded) return;
		s_Music[(int)BgmTrack::Stage1] = CreateLoopBgm(128, 220, 8, 0, 0.25f);
		s_Music[(int)BgmTrack::Stage2] = CreateLoopBgm(140, 196, 8, 1, 0.3f);
		s_Music[(int)BgmTrack::MidBoss] = CreateLoopBgm(150, 165, 8, 1, 0.35f);
		s_Music[(int)BgmTrack::Boss] = CreateLoopBgm(160, 147, 8, 1, 0.4f);
		s_Loaded = 1;
	}

	void Final()
	{
		Stop();
		for (int i = 1; i <= 4; i++)
		{
			if (s_Music[i] != 0)
			{
				DeleteSoundMem(s_Music[i]);
				s_Music[i] = 0;
			}
		}
		s_Loaded = 0;
	}

	void Play(BgmTrack track)
	{
		if (track == s_Current) return;
		Stop();
		int idx = (int)track;
		if (idx < 1 || idx > 4 || s_Music[idx] == 0) return;
		SetVolumeSoundMem(GameOptionsGetBgmVolume255(), s_Music[idx]);
		PlaySoundMem(s_Music[idx], DX_PLAYTYPE_LOOP);
		s_Current = track;
	}

	void Stop()
	{
		if (s_Current != BgmTrack::None)
		{
			int idx = (int)s_Current;
			if (s_Music[idx] != 0)
				StopSoundMem(s_Music[idx]);
		}
		s_Current = BgmTrack::None;
	}

	void Update()
	{
		if (s_Current == BgmTrack::None) return;
		int idx = (int)s_Current;
		if (s_Music[idx] != 0)
			SetVolumeSoundMem(GameOptionsGetBgmVolume255(), s_Music[idx]);
	}

	BgmTrack GetCurrentTrack() { return s_Current; }
}
