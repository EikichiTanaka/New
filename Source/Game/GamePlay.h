#pragma once

#include "GameConfig.h"
#include "Game/BlockManager.h"
#include "Game/Paddle.h"
#include "Game/Ball.h"
#include "Game/GhostRecorder.h"
#include "Game/GameEffect.h"

// Full block-breaker play logic for GameScene
class GamePlay
{
public:
	void Init(Difficulty diff);
	void Update(bool enterKey, bool pauseKey);
	void Draw() const;
	bool IsFinished() const { return m_Done; }

private:
	enum class State { Ready, Playing, Paused };

	void ResetBall();
	void EndPlay(bool clear, bool stalemate, bool gameOver);
	void DrawBackground() const;
	void DrawHud() const;
	const char* GetDifficultyName() const;

	BlockManager m_Blocks;
	Paddle m_Paddle;
	Ball m_Ball;
	GhostRecorder m_Ghost;
	GameEffect m_Effect;

	Difficulty m_Difficulty = Difficulty::Normal;
	State m_State = State::Ready;
	int m_Lives = START_LIVES;
	int m_NoBreakTimer = 0;
	int m_PlayFrames = 0;
	int m_Combo = 0;
	int m_ComboTimer = 0;
	int m_MatrixAnim = 0;
	int m_HitstopTimer = 0;
	
	int m_SmashWindowTimer = 0;
	int m_SmashCooldown = 0;
	int m_SmashTextTimer = 0;
	int m_ReplenishTextTimer = 0;
	
	bool m_Done = false;
};
