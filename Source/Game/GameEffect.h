#pragma once

// Simple visual effects (block break sparks, miss flash)
class GameEffect
{
public:
	void Clear();
	void AddBlockBreak(float x, float y, bool isSuperBall);
	void AddScorePopup(float x, float y, int score, int combo);
	void AddLifeLostFlash();
	void TriggerShake(int intensity, int durationFrames);
	void TriggerGlitch(int durationFrames);
	void Update();
	void Draw() const;

private:
	struct Particle
	{
		float x, y;
		float vx, vy;
		int life;
		int color;
		bool active;
	};

	static const int PARTICLE_MAX = 96;
	Particle m_Particles[PARTICLE_MAX] = {};

	struct PopupText
	{
		float x, y;
		float vy;
		int life;
		int score;
		int combo;
		bool active;
	};
	static const int POPUP_MAX = 32;
	PopupText m_Popups[POPUP_MAX] = {};

	int m_FlashTimer = 0;
	
	int m_ShakeTimer = 0;
	int m_ShakeIntensity = 0;
	
	int m_GlitchTimer = 0;
};
