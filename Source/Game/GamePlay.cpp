#include "Game/GamePlay.h"
#include "Common/GameData.h"
#include "Common/KeyHelper.h"
#include "Common/SoundSynth.h"
#include "DxLib.h"
#include <math.h>

void GamePlay::Init(Difficulty diff)
{
	m_Difficulty = diff;
	m_Done = false;
	m_State = State::Ready;
	m_Lives = START_LIVES;
	m_NoBreakTimer = 0;
	m_PlayFrames = 0;
	m_Combo = 0;
	m_ComboTimer = 0;
	m_MatrixAnim = 0;
	m_HitstopTimer = 0;
	m_SmashWindowTimer = 0;
	m_SmashCooldown = 0;
	m_SmashTextTimer = 0;
	m_ReplenishTextTimer = 0;

	g_GameData.score = 0;
	g_GameData.blocksBroken = 0;
	g_GameData.livesLeft = m_Lives;
	g_GameData.maxCombo = 0;
	g_GameData.playTimeSec = 0;
	g_GameData.isClear = false;
	g_GameData.isStalemate = false;
	g_GameData.isGameOver = false;

	m_Blocks.Init(diff);
	m_Paddle.Init(diff);
	m_Ball.Init(diff);
	m_Ghost.Init(diff);
	m_Effect.Clear();
	ResetBall();
}

void GamePlay::ResetBall()
{
	m_Ball.ResetOnPaddle(m_Paddle);
	m_Ball.SetSuperBall(false);
	m_State = State::Ready;
	m_NoBreakTimer = 0;
}

void GamePlay::EndPlay(bool clear, bool stalemate, bool gameOver)
{
	m_Done = true;
	g_GameData.isClear = clear;
	g_GameData.isStalemate = stalemate;
	g_GameData.isGameOver = gameOver;
	g_GameData.playTimeSec = m_PlayFrames / 60;
	if (clear) g_GameData.score += SCORE_BONUS_CLEAR;
}

const char* GamePlay::GetDifficultyName() const
{
	switch (m_Difficulty)
	{
	case Difficulty::Easy:   return "EASY";
	case Difficulty::Hard:   return "HARD";
	default:                 return "NORMAL";
	}
}

void GamePlay::Update(bool enterKey, bool pauseKey)
{
	if (m_Done) return;

	m_MatrixAnim++;
	m_Effect.Update();
	
	if (m_HitstopTimer > 0)
	{
		m_HitstopTimer--;
		return; // Freeze physics during hitstop
	}

	if (pauseKey && m_State == State::Playing)
	{
		m_State = State::Paused;
		return;
	}
	if (pauseKey && m_State == State::Paused)
	{
		m_State = State::Playing;
		return;
	}

	if (m_State == State::Paused)
		return;

	m_Paddle.Update();
	m_Ghost.Record(m_Paddle.GetCenterX(), m_Paddle.GetWidth());

	if (m_State == State::Ready)
	{
		if (enterKey)
		{
			m_Ball.Launch();
			m_State = State::Playing;
			m_NoBreakTimer = 0;
		}
		m_Ball.Update(m_Paddle, 0, 0, 0, 0, false, false);
		return;
	}

	m_PlayFrames++;

	if (m_SmashCooldown > 0) m_SmashCooldown--;
	if (m_SmashWindowTimer > 0) m_SmashWindowTimer--;
	if (m_SmashTextTimer > 0) m_SmashTextTimer--;

	if (KeyHelper::IsTrigger(KEY_INPUT_SPACE) && m_SmashCooldown == 0)
	{
		m_SmashWindowTimer = 15; // Increased to 15 frames for easier timing
		m_SmashCooldown = 20; // Reduced cooldown
	}

	float gL, gR, gT, gB;
	bool ghostOn = false;
	m_Ghost.GetMainGhost(gL, gR, gT, gB, ghostOn);
	
	bool smashSuccess = m_Ball.Update(m_Paddle, gL, gR, gT, gB, ghostOn, m_SmashWindowTimer > 0);
	
	if (smashSuccess)
	{
		m_HitstopTimer = 6;
		m_Paddle.TriggerSmashEffect();
		m_Effect.TriggerGlitch(12); // Smash glitch overlay
		m_SmashTextTimer = 30;
		m_Combo = COMBO_SUPER_BALL_THRESHOLD; // Max combo instantly for piercing
	}

	int add = 0;
	BlockHitSide side = BlockHitSide::Top;
	if (m_Blocks.HitBall(m_Ball.GetX(), m_Ball.GetY(), BALL_RADIUS, add, side, m_Effect, m_Combo))
	{
		if (m_ComboTimer > 0) m_Combo++;
		else m_Combo = 1;
		m_ComboTimer = COMBO_TIMEOUT_FRAMES;
		if (m_Combo > g_GameData.maxCombo) g_GameData.maxCombo = m_Combo;

		int comboBonus = (m_Combo - 1) * COMBO_SCORE_BONUS;
		int totalAdd = add + comboBonus;
		g_GameData.score += totalAdd;
		g_GameData.blocksBroken++;
		m_NoBreakTimer = 0;

		m_Ball.ReflectBlock(side);
		m_Effect.AddBlockBreak(m_Ball.GetX(), m_Ball.GetY(), m_Ball.IsSuperBall());
		m_Effect.AddScorePopup(m_Ball.GetX(), m_Ball.GetY(), totalAdd, m_Combo);
		
		SoundSynth::PlayBlockBreak(m_Combo);
		
		// Juice: Screen shake and hitstop
		if (m_Ball.IsSuperBall()) {
			m_HitstopTimer = 0; // No hitstop while piercing
		} else {
			m_HitstopTimer = m_Combo >= COMBO_SUPER_BALL_THRESHOLD ? 4 : 2;
		}
		m_Effect.TriggerShake(m_Combo * 2 + 5, 10);
		
		if (m_Combo >= COMBO_SUPER_BALL_THRESHOLD)
		{
			m_Ball.SetSuperBall(true);
		}
	}
	else
	{
		if (m_ComboTimer > 0) m_ComboTimer--;
		else
		{
			m_Combo = 0;
			// SuperBall now persists beautifully until the ball bounces on the paddle again!
		}
	}

	if (m_Ball.IsOutBottom())
	{
		m_Lives--;
		g_GameData.livesLeft = m_Lives;
		m_Effect.AddLifeLostFlash();
		m_Combo = 0;
		m_ComboTimer = 0;

		if (m_Lives <= 0)
		{
			EndPlay(false, false, true);
			return;
		}
		ResetBall();
		return;
	}

	if (m_ReplenishTextTimer > 0) m_ReplenishTextTimer--;

	// Endless mode: Check if blocks are running low (<= 8 remaining blocks)
	if (m_Blocks.GetRemain() <= 8 && m_ReplenishTextTimer == 0)
	{
		m_Blocks.Replenish(m_Difficulty, m_Effect);
		m_ReplenishTextTimer = 90; // Alert duration (1.5 seconds)
		m_Effect.TriggerGlitch(15);
		SoundSynth::PlaySmash(); // Massive sub-bass sweep sound
	}

	if (m_Blocks.IsAllClear())
	{
		EndPlay(true, false, false);
		return;
	}

	m_NoBreakTimer++;
	if (m_Blocks.GetRemain() > 0 && m_NoBreakTimer >= STALEMATE_SECONDS * 60)
	{
		EndPlay(false, true, false);
	}
}

void GamePlay::DrawBackground() const
{
	for (int x = 0; x < 16; x++)
	{
		int col = (x + m_MatrixAnim / 4) % 16;
		char ch = '0' + (col % 10);
		DrawFormatString(40 + x * 76, 8 + (col * 7) % 30, GetColor(0, 40 + col * 8, 0), "%c", ch);
	}

	DrawBox(FIELD_LEFT - 3, FIELD_TOP - 3, FIELD_RIGHT + 3, FIELD_BOTTOM + 3, GetColor(0, 255, 100), FALSE);
	DrawBox(FIELD_LEFT, FIELD_TOP, FIELD_RIGHT, FIELD_BOTTOM, GetColor(4, 10, 18), TRUE);

	for (int y = FIELD_TOP; y < FIELD_BOTTOM; y += 30)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 22);
		DrawLine(FIELD_LEFT, y, FIELD_RIGHT, y, GetColor(0, 60, 30));
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}
}

void GamePlay::DrawHud() const
{
	DrawFormatString(FIELD_LEFT, 18, GetColor(0, 255, 120),
		"SCORE:%d  BLOCK:%d/%d  LIFE:%d  COMBO:x%d",
		g_GameData.score, m_Blocks.GetRemain(), m_Blocks.GetTotalAtStart(),
		m_Lives, m_Combo > 1 ? m_Combo : 0);

	DrawFormatString(FIELD_RIGHT - 120, 18, GetColor(0, 200, 255),
		"MODE:%s", GetDifficultyName());

	int centerX = SCREEN_WIDTH / 2;

	if (m_State == State::Ready && !m_Done)
	{
		DrawFormatString(centerX - 136, SCREEN_HEIGHT / 2 + 40, GetColor(255, 255, 0),
			">> ENTER / SPACE : Launch Ball <<");
		DrawFormatString(centerX - 160, SCREEN_HEIGHT / 2 + 75, GetColor(200, 200, 200),
			"Move paddle with Left/Right or A/D first");
	}

	if (m_State == State::Playing && !m_Done)
	{
		int sec = STALEMATE_SECONDS - m_NoBreakTimer / 60;
		if (sec <= 5 && m_Blocks.GetRemain() > 0)
		{
			DrawFormatString(centerX - 110, SCREEN_HEIGHT / 2 + 40, GetColor(255, 80, 80),
				"!! STALEMATE WARNING %d sec !!", sec > 0 ? sec : 0);
		}

		// Draw Smash Zone Indicator (Collapsing Ring) in 2D
		if (m_Ball.GetVY() > 0)
		{
			float dist = m_Paddle.GetTop() - (m_Ball.GetY() + BALL_RADIUS);
			if (dist >= -20.f && dist <= 150.f)
			{
				float progress = 1.0f - (dist / 150.f);
				if (progress < 0.0f) progress = 0.0f;
				if (progress > 1.0f) progress = 1.0f;
				
				float ringRadius = BALL_RADIUS + (1.0f - progress) * 80.f;
				int alpha = 100 + (int)(progress * 155);
				
				int sx = (int)m_Ball.GetX();
				int sy = (int)m_Ball.GetY();

				SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
				DrawCircle(sx, sy, (int)ringRadius, GetColor(0, 255, 255), FALSE);
				DrawCircle(sx, sy, (int)ringRadius + 1, GetColor(0, 255, 255), FALSE);
				
				// Perfect timing zone indicator
				if (dist >= -20.f && dist <= 50.f)
				{
					DrawCircle(sx, sy, (int)BALL_RADIUS + 4, GetColor(255, 255, 0), FALSE);
				}
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
		}
	}

	if (m_SmashTextTimer > 0)
	{
		int x = (int)m_Paddle.GetCenterX() - 40;
		int y = (int)m_Paddle.GetTop() - 30;

		if (m_SmashTextTimer > 18) // During initial high-intensity glitch
		{
			SetDrawBlendMode(DX_BLENDMODE_ADD, m_SmashTextTimer * 8);
			DrawFormatString(x - 4, y, GetColor(255, 0, 100), "SMASH!!");
			DrawFormatString(x + 4, y, GetColor(0, 255, 255), "SMASH!!");
		}

		SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_SmashTextTimer * 8);
		DrawFormatString(x, y, GetColor(255, 255, 0), "SMASH!!");
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	if (m_State == State::Paused)
	{
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
		DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(0, 0, 0), TRUE);
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
		DrawFormatString(SCREEN_WIDTH / 2 - 40, SCREEN_HEIGHT / 2 - 20, GetColor(255, 255, 0), "PAUSED");
		DrawFormatString(SCREEN_WIDTH / 2 - 110, SCREEN_HEIGHT / 2 + 20, GetColor(200, 200, 200),
			"Press P to resume");
	}

	if (m_ReplenishTextTimer > 0)
	{
		int pulse = 100 + (int)(sinf(m_PlayFrames * 0.2f) * 155);
		if (pulse < 0) pulse = 0;
		if (pulse > 255) pulse = 255;
		SetDrawBlendMode(DX_BLENDMODE_ALPHA, pulse);
		int textX = SCREEN_WIDTH / 2 - 200;
		int textY = SCREEN_HEIGHT / 2 - 40;
		
		// Tech-glowing border and text
		DrawBox(textX - 20, textY - 15, textX + 420, textY + 45, GetColor(0, 255, 100), FALSE);
		DrawBox(textX - 22, textY - 17, textX + 422, textY + 47, GetColor(0, 100, 50), FALSE);
		
		DrawFormatString(textX, textY + 5, GetColor(0, 255, 100), "SYS ALERT: BLOCKS REPLENISHED");
		SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	}

	DrawFormatString(FIELD_LEFT, SCREEN_HEIGHT - 32, GetColor(100, 100, 100),
		"P:Pause  |  Purple GHOST = your past paddle (OBSTACLE blocking shots)");
}

void GamePlay::Draw() const
{
	// 1. Draw flat dark background
	DrawBox(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GetColor(5, 10, 18), TRUE);
	
	// 2. Draw 2D background with neon grid
	DrawBackground();

	// 3. Draw game elements in standard 2D
	m_Blocks.Draw();
	m_Ghost.Draw();
	m_Paddle.Draw();
	m_Ball.Draw();
	m_Effect.Draw();

	// 4. Draw overlay HUD
	DrawHud();
}
