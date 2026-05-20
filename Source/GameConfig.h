#pragma once

// GameConfig.h : screen size and game rules

constexpr int SCREEN_WIDTH = 1920;
constexpr int SCREEN_HEIGHT = 1080;

static const char* WINDOW_TITLE = "Hacker Brain - Block Breaker";

enum class Difficulty
{
	Easy = 0,
	Normal = 1,
	Hard = 2
};

constexpr int FIELD_LEFT = 120;
constexpr int FIELD_TOP = 80;
constexpr int FIELD_RIGHT = 1800;
constexpr int FIELD_BOTTOM = 1000;

constexpr int BLOCK_COLS = 20;
constexpr int BLOCK_ROWS = 8;
constexpr int BLOCK_WIDTH = 76;
constexpr int BLOCK_HEIGHT = 22;
constexpr int BLOCK_GAP_X = 7;
constexpr int BLOCK_GAP_Y = 7;
constexpr int BLOCK_START_X = FIELD_LEFT + 22;
constexpr int BLOCK_START_Y = FIELD_TOP + 40;

constexpr int PADDLE_WIDTH_EASY = 180; // slightly wider paddle since field is wider
constexpr int PADDLE_WIDTH_NORMAL = 135;
constexpr int PADDLE_WIDTH_HARD = 100;
constexpr int PADDLE_HEIGHT = 18;
constexpr int PADDLE_Y = FIELD_BOTTOM - 60;
constexpr float PADDLE_SPEED = 9.0f;

constexpr float BALL_RADIUS = 8.0f;
constexpr float BALL_SPEED_EASY = 5.0f;
constexpr float BALL_SPEED_NORMAL = 6.5f;
constexpr float BALL_SPEED_HARD = 8.0f;
constexpr float BALL_SPEED_MAX = 11.0f;
constexpr float BALL_STEER_FORCE = 0.25f; // Strength of mid-air steering
constexpr int TRAIL_MAX = 16; // Length of the ball trail

constexpr int GHOST_DELAY_EASY = 90;
constexpr int GHOST_DELAY_NORMAL = 45;
constexpr int GHOST_DELAY_HARD = 20;
constexpr int GHOST_TRAIL_DRAW = 3;
constexpr int GHOST_HISTORY_MAX = 128;

constexpr int START_LIVES = 3;
constexpr int SCORE_PER_BLOCK = 100;
constexpr int SCORE_BONUS_CLEAR = 1000;
constexpr int STALEMATE_SECONDS = 12;
constexpr int COMBO_TIMEOUT_FRAMES = 90;
constexpr int COMBO_SCORE_BONUS = 25;
constexpr int COMBO_SUPER_BALL_THRESHOLD = 5; // Combo needed for Super Ball
