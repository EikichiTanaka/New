#pragma once

enum class BgmTrack
{
	None = 0,
	Stage1,
	Stage2,
	MidBoss,
	Boss
};

namespace BgmPlayer
{
	void Init();
	void Final();
	void Play(BgmTrack track);
	void Stop();
	void Update();
	BgmTrack GetCurrentTrack();
}
