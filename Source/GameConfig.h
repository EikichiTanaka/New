#pragma once

// ============================================================
// GameConfig.h
// ゲーム全体で使用する定数・設定
// ============================================================

#include "Common/GameScreen.h"

static const char* WINDOW_TITLE = "Bullet Storm - 3D Danmaku Shooter";

enum class Difficulty
{
	Easy   = 0,
	Normal = 1,
	Hard   = 2,
	Lunatic = 3
};

constexpr int STAGE_CHAPTER_COUNT = 2;

constexpr float FIELD_WIDTH   = 800.0f;
constexpr float FIELD_HEIGHT  = 600.0f;
constexpr float FIELD_DEPTH   = 1200.0f;
constexpr float FIELD_HALF_W  = FIELD_WIDTH / 2.0f;
constexpr float FIELD_HALF_D  = FIELD_DEPTH / 2.0f;

constexpr float PLAYER_SPEED       = 7.0f;
constexpr float PLAYER_SLOW_SPEED  = 2.5f;
constexpr float PLAYER_HITBOX      = 4.0f;
constexpr float PLAYER_DRAW_SIZE   = 12.0f;
constexpr float PLAYER_Y           = 15.0f;
constexpr float PLAYER_START_X     = 0.0f;
constexpr float PLAYER_START_Z     = -350.0f;
constexpr int   PLAYER_FIRE_RATE   = 2;
constexpr int   PLAYER_INV_FRAMES  = 90;
constexpr int   START_LIVES        = 3;

constexpr float PBULLET_SPEED      = 24.0f;
constexpr float PBULLET_RADIUS     = 3.5f;
constexpr int   PBULLET_MAX        = 128;

constexpr float EBULLET_SPEED_EASY    = 3.2f;
constexpr float EBULLET_SPEED_NORMAL  = 5.2f;
constexpr float EBULLET_SPEED_HARD    = 6.8f;
constexpr float EBULLET_SPEED_LUNATIC = 8.2f;
constexpr float EBULLET_RADIUS       = 4.5f;
constexpr int   EBULLET_MAX              = 512;
constexpr int   MAX_ACTIVE_ENEMY_BULLETS = 160;

// フィーバー（弾幕カグラ風：撃破チェイン＋かすりでゲージ蓄積）
constexpr float FEVER_GAUGE_MAX           = 100.0f;
constexpr float FEVER_GAUGE_PER_KILL      = 14.0f;
constexpr float FEVER_GAUGE_PER_GRAZE       = 1.2f;
constexpr float FEVER_GAUGE_PER_BOSS_HIT    = 4.0f;
constexpr float FEVER_GAUGE_HIT_PENALTY     = 50.0f;
constexpr int   FEVER_KILL_CHAIN_NEED       = 6;
constexpr int   FEVER_KILL_CHAIN_WINDOW     = 180;
constexpr float FEVER_GAUGE_CHAIN_BONUS     = 22.0f;
constexpr int   FEVER_GRAZE_RUSH_NEED       = 12;
constexpr int   FEVER_GRAZE_RUSH_WINDOW     = 90;
constexpr float FEVER_GAUGE_GRAZE_RUSH_BONUS = 18.0f;
constexpr int   FEVER_DURATION_FRAMES       = 480;
constexpr int   FEVER_BONUS_SCORE           = 3000;

constexpr int   ENEMY_MAX          = 32;
constexpr float ENEMY_DRAW_SIZE    = 18.0f;
constexpr int   SCORE_PER_ENEMY    = 150;
constexpr int   SCORE_GRAZE        = 10;
constexpr float GRAZE_RADIUS       = 20.0f;

constexpr int   MAX_WAVES          = 5;

constexpr int   PARTICLE_MAX       = 200;

constexpr int   SPELL_BREAK_FRAMES = 150;
constexpr int   BOSS_INTRO_FRAMES  = 150;
constexpr int   CHARGE_MAX_FRAMES  = 45;
constexpr float CHARGE_SHOT_RADIUS_MUL = 2.8f;
constexpr int   SPELL_BONUS_SCORE    = 220;
constexpr float SCORE_BONUS_NO_MISS  = 1.15f;
constexpr float SCORE_BONUS_NO_BOMB  = 1.10f;
constexpr int   MIDBOSS_HP_1       = 55;
constexpr int   MIDBOSS_HP_2       = 75;
constexpr float FEVER_GAUGE_READY  = 80.0f;
constexpr int   PIERCE_HIT_MAX     = 4;
constexpr int   HITSTOP_FRAMES           = 3;
constexpr int   ENEMY_HIT_IFRAMES        = 5;
constexpr int   BOSS_HIT_IFRAMES         = 2;
constexpr int   MIDBOSS_HIT_IFRAMES      = 2;
constexpr int   MAX_HIT_SPARKLES_FRAME       = 4;
constexpr int   BOSS_HIT_FX_COOLDOWN         = 4;
constexpr int   BOSS_BULLET_COLLISION_SKIP   = 2;
constexpr int   MAX_BOSS_BULLET_RELEASES_FRAME = 32;

constexpr int   SPATIAL_GRID_COLS        = 12;
constexpr int   SPATIAL_GRID_ROWS        = 16;
constexpr int   SHOW_FPS           = 1;

// 軽量3D敵弾（無ライト・低セグ・自機基準XZカリング）
constexpr int   FIELD_GRID_SPACING            = 160;
constexpr int   BULLET_DRAW_SEG_ENEMY         = 4;
constexpr int   BULLET_DRAW_SEG_PLAYER        = 6;
constexpr float BULLET_DRAW_CULL_DIST_Z       = 420.0f;
constexpr float BULLET_DRAW_CULL_DIST_X       = 420.0f;

// ウェーブ開始直後は弾を出さない／間隔を空けて同時弾数を抑える
constexpr int   WAVE_SPAWN_DELAY_FRAMES       = 120;
constexpr int   WAVE_DANMAKU_WARMUP_FRAMES    = 120;
constexpr int   WAVE_DANMAKU_FRAME_INTERVAL   = 4;

constexpr int   EFFECT_MAX_DRAW_PARTICLES     = 28;
constexpr int   EFFECT_STAR_DRAW_MAX          = 24;
constexpr int   EXPLOSION_PARTICLE_COUNT    = 8;
constexpr int   GRAZE_SPARK_COUNT           = 2;

inline float DifficultyBulletSpeed(Difficulty d)
{
	switch (d)
	{
	case Difficulty::Easy:    return EBULLET_SPEED_EASY;
	case Difficulty::Hard:    return EBULLET_SPEED_HARD;
	case Difficulty::Lunatic: return EBULLET_SPEED_LUNATIC;
	default:                  return EBULLET_SPEED_NORMAL;
	}
}

inline float DifficultyDanmakuIntervalMul(Difficulty d)
{
	switch (d)
	{
	case Difficulty::Easy:    return 1.55f;
	case Difficulty::Hard:    return 0.72f;
	case Difficulty::Lunatic: return 0.58f;
	default:                  return 1.0f;
	}
}

inline float DifficultyBulletRadiusMul(Difficulty d)
{
	switch (d)
	{
	case Difficulty::Easy:    return 0.78f;
	case Difficulty::Hard:    return 1.08f;
	case Difficulty::Lunatic: return 1.15f;
	default:                  return 1.0f;
	}
}

inline int DifficultyCount() { return 4; }

