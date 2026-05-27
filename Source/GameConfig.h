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
// 自機：見た目(MV1)は PLAYER_DRAW_SIZE、被弾は PLAYER_COLLISION_RADIUS（小さめの芯）
constexpr float PLAYER_COLLISION_RADIUS = 4.3f;
constexpr float PLAYER_HITBOX           = PLAYER_COLLISION_RADIUS; // 互換
constexpr float PLAYER_DRAW_SIZE        = 18.0f;
constexpr float PLAYER_Y           = 15.0f;
constexpr float PLAYER_START_X     = 0.0f;
constexpr float PLAYER_START_Z     = -350.0f;
constexpr int   PLAYER_FIRE_RATE   = 2;
constexpr int   PLAYER_INV_FRAMES  = 90;
constexpr int   START_LIVES        = 3;
constexpr int   START_BOMBS        = 3;
constexpr int   BOMB_STOCK_MAX     = 8;
constexpr int   BOMB_PHASE_CLEAR_BONUS = 1;

constexpr float PBULLET_SPEED      = 24.0f;
constexpr float PBULLET_RADIUS     = 5.5f;
constexpr int   PBULLET_MAX        = 128;

constexpr float EBULLET_SPEED_EASY    = 3.0f;
constexpr float EBULLET_SPEED_NORMAL  = 4.6f;
constexpr float EBULLET_SPEED_HARD    = 6.2f;
constexpr float EBULLET_SPEED_LUNATIC = 7.6f;
constexpr float EBULLET_RADIUS       = 6.0f;
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
constexpr float PLAYER_GRAZE_RADIUS = 16.0f;
constexpr float GRAZE_RADIUS        = PLAYER_GRAZE_RADIUS; // 互換

constexpr int   MAX_WAVES          = 5;

constexpr int   PARTICLE_MAX       = 200;

constexpr int   SPELL_BREAK_FRAMES = 150;
constexpr int   BOSS_INTRO_FRAMES  = 150;

// ============================================================
// [BOSS_RUSH_MODE] ステージを廃して開幕からボス1体と戦う形式
// true  : ザコ・中ボス・ウェーブを全てスキップ、ボスは多フェーズ＆HP大
// false : 従来のステージ形式（ウェーブ→中ボス→ボス）
// ============================================================
constexpr bool  BOSS_RUSH_MODE                = true;
constexpr int   BOSS_RUSH_PHASE_COUNT         = 4;
// 各フェーズの基準HP（難易度倍率は GetBossRushPhaseHp で適用）
constexpr int   BOSS_RUSH_PHASE_HP_BASE[BOSS_RUSH_PHASE_COUNT] = { 260, 340, 440, 560 };

// ボスラッシュ専用：難易度ごとの弾幕・体力調整（ここを編集）
struct BossRushDifficultyTune
{
	float hpMul;
	float bulletSpeedMul;
	float danmakuIntervalMul;  // 大きいほど発射間隔が長い（緩い）
	float bulletCountMul;
	float bulletRadiusMul;
	int   fireIntervalBonus;   // UpdateBoss の interval に加算（大きいほど遅い）
};

constexpr BossRushDifficultyTune BOSS_RUSH_TUNE[4] = {
	// Easy
	{ 0.72f, 0.80f, 1.48f, 0.72f, 0.72f, 4 },
	// Normal
	{ 1.00f, 0.88f, 1.18f, 0.86f, 0.86f, 3 },
	// Hard
	{ 1.18f, 1.02f, 0.92f, 1.02f, 0.96f, 1 },
	// Lunatic
	{ 1.35f, 1.10f, 0.76f, 1.12f, 1.02f, 0 }
};
constexpr int   CHARGE_MAX_FRAMES        = 45;
constexpr int   CHARGE_MIN_FRAMES        = 10;
constexpr float CHARGE_SHOT_RADIUS_MUL   = 4.2f;
constexpr int   CHARGE_SHOT_DAMAGE_BASE  = 5;
constexpr int   CHARGE_SHOT_DAMAGE_SCALE = 10;
constexpr int   CHARGE_SHOT_PIERCE_BONUS = 10;
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

// ============================================================
// [VISUAL_RICH] 見た目強化スイッチ
// true  : 派手なネオン演出（加算発光・多層描画・スクロール床・星空）
// false : 完全軽量化（最小限の描画のみ）
// 重さが気になったらこの 1 行を false に戻すだけで軽量版に戻せます。
// 各実装箇所には // [VISUAL_RICH] というコメントが付いています。
// ============================================================
constexpr bool  VISUAL_RICH                   = false;

// [BACKGROUND_FLASHY] 背景演出（弾幕本編。VISUAL_RICH とは独立して床・空だけ派手にできる）
constexpr bool  BACKGROUND_FLASHY_ENABLED         = true;
constexpr int   BACKGROUND_FLASHY_STAR_COUNT      = 96;
constexpr int   BACKGROUND_FLASHY_NEBULA_COUNT    = 8;
constexpr int   BACKGROUND_FLASHY_METEOR_COUNT      = 6;
constexpr int   BACKGROUND_FLASHY_AMBIENT_BLOBS   = 8;
constexpr int   BACKGROUND_FLASHY_GRID_SCROLL_SPEED = 4;
constexpr bool  BACKGROUND_FLASHY_FIELD_GRID      = true;
constexpr bool  BACKGROUND_FLASHY_AMBIENT_3D      = true;

// [BACKGROUND_LITE] オプション「背景軽量」ON 時（背景のみ・弾幕/HUD はそのまま）
constexpr int   BACKGROUND_LITE_STAR_COUNT      = 32;
constexpr int   BACKGROUND_LITE_NEBULA_COUNT    = 2;
constexpr int   BACKGROUND_LITE_AMBIENT_BLOBS   = 3;
constexpr int   BACKGROUND_LITE_GRID_SCROLL_SPEED = 2;
constexpr bool  BACKGROUND_LITE_FIELD_GRID      = true;
constexpr bool  BACKGROUND_LITE_AMBIENT_3D      = true;


// スプライト描画（ResourceManager）。true=ビルボード画像、false=球/シルエット
constexpr bool  USE_GAME_SPRITES              = true;

// ============================================================
// [MV1_MODEL] 3Dモデル（.mv1）のパスと表示調整
// 大きさ・位置・向きは GameConfig.h の値だけで変更できます。
// ============================================================
struct ModelDrawSettings
{
	float scaleX;
	float scaleY;
	float scaleZ;
	float offsetX;
	float offsetY;
	float offsetZ;
	float rotXDeg;
	float rotYDeg;
	float rotZDeg;
	float radiusRef;       // 0 より大: scale に (ゲーム内半径 / radiusRef) を乗算
	float radiusScaleMul;  // 上記の追加倍率（敵タイプ別など）
};

// 自機モデル（assets/models/PlayerShip/ など）
constexpr const char* PLAYER_MODEL_PATH_PRIMARY = "assets/models/PlayerShip/Fighter 38.mv1";
constexpr const char* PLAYER_MODEL_PATH_FALLBACK = "assets/models/player.mv1";

// 雑魚敵モデル（ボス戦以外）
constexpr const char* ENEMY_MODEL_PATH_PRIMARY = "assets/models/EnemyShip/Fighter 59.mv1";
constexpr const char* ENEMY_MODEL_PATH_FALLBACK = "assets/models/EnemyShip2/Drone.mv1";

// ボス第1〜4形態（phase 0〜3）ごとの MV1。EnemyShip / EnemyShip2 / … と対応
constexpr int BOSS_PHASE_MODEL_COUNT = BOSS_RUSH_PHASE_COUNT;
constexpr const char* BOSS_PHASE_MODEL_PATHS[BOSS_PHASE_MODEL_COUNT] = {
	"assets/models/EnemyShip/Fighter 59.mv1",
	"assets/models/EnemyShip2/Drone.mv1",
	"assets/models/EnemyShip3/Fighter 232.mv1",
	"assets/models/EnemyShip4/uploads_files_4254779_SpaceShip.mv1"
};

// --- 表示調整（合わないときはここだけ編集）---
constexpr ModelDrawSettings PLAYER_MV1_SETTINGS = {
	15.5f, 15.5f, 15.5f,
	0.0f, 0.0f, 0.0f,
	0.0f, 90.0f, 0.0f,
	0.0f, 1.0f
};

constexpr ModelDrawSettings ENEMY_MV1_SETTINGS = {
	1.0f, 1.0f, 1.0f,
	0.0f, 0.0f, 0.0f,
	0.0f, 180.0f, 0.0f,
	ENEMY_DRAW_SIZE, 1.0f
};

// ボス第1〜4形態それぞれのサイズ・向き（個別にずらせます）
constexpr ModelDrawSettings BOSS_PHASE_MV1_SETTINGS[BOSS_PHASE_MODEL_COUNT] = {
	// 第1形態 (phase 0)
	{ 20.0f, 20.0f, 20.0f,  0.0f, 0.0f, 0.0f,  0.0f, 90.0f, 0.0f,  40.0f, 1.0f },
	// 第2形態 (phase 1)
	{ 0.2f, 0.2f, 0.2f,  0.0f, 0.0f, 0.0f,  0.0f, 180.0f, 0.0f,  40.0f, 1.0f },
	// 第3形態 (phase 2)
	{ 2.5f, 2.5f, 2.5f,  0.0f, 0.0f, 0.0f,  0.0f, -90.0f, 0.0f,  42.0f, 1.0f },
	// 第4形態 (phase 3)
	{ 0.07f, 0.07f, 0.07f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 0.0f,  44.0f, 1.05f }
};

inline const ModelDrawSettings& GetBossPhaseModelSettings(int phase)
{
	if (phase < 0) phase = 0;
	if (phase >= BOSS_PHASE_MODEL_COUNT) phase = BOSS_PHASE_MODEL_COUNT - 1;
	return BOSS_PHASE_MV1_SETTINGS[phase];
}

constexpr ModelDrawSettings MIDBOSS_MV1_SETTINGS = {
	2.8f, 2.8f, 2.8f,
	0.0f, 0.0f, 0.0f,
	0.0f, 180.0f, 0.0f,
	28.0f, 1.0f
};

// ============================================================
// [COLLISION] 当たり判定（見た目スケールとは別。ここで調整）
// 弾：発射時の logical 半径 → 衝突半径 / 描画半径 に変換
// ============================================================
constexpr float ENEMY_GRUNT_COLLISION_RADIUS = 20.0f;
constexpr float MIDBOSS_DRAW_RADIUS_REF      = 28.0f;
constexpr float MIDBOSS_COLLISION_RADIUS     = 30.0f;
constexpr float BOSS_DRAW_RADIUS_REF         = 44.0f;

// ボス形態ごとの被弾範囲（MV1スケールに合わせて個別調整）
constexpr float BOSS_PHASE_COLLISION_RADIUS[BOSS_PHASE_MODEL_COUNT] = {
	40.0f,  // 第1形態（大型スケール）
	35.0f,  // 第2形態（小型）
	42.0f,  // 第3形態
	34.0f   // 第4形態
};

constexpr float ENEMY_BULLET_VISUAL_MUL      = 4.8f;
constexpr float ENEMY_BULLET_COLLISION_MUL   = 1.72f;
constexpr float PLAYER_BULLET_VISUAL_MUL     = 5.2f;
constexpr float PLAYER_BULLET_COLLISION_MUL  = 2.4f;

inline float EnemyBulletCollisionRadius(float logicalRadius)
{
	return logicalRadius * ENEMY_BULLET_COLLISION_MUL;
}

inline float EnemyBulletVisualRadius(float logicalRadius)
{
	return logicalRadius * ENEMY_BULLET_VISUAL_MUL;
}

inline float EnemyBulletVisualRadiusFromCollision(float collisionRadius)
{
	return collisionRadius * (ENEMY_BULLET_VISUAL_MUL / ENEMY_BULLET_COLLISION_MUL);
}

inline float PlayerBulletCollisionRadius(float logicalRadius)
{
	return logicalRadius * PLAYER_BULLET_COLLISION_MUL;
}

inline float PlayerBulletVisualRadius(float logicalRadius)
{
	return logicalRadius * PLAYER_BULLET_VISUAL_MUL;
}

inline float PlayerBulletVisualRadiusFromCollision(float collisionRadius)
{
	return collisionRadius * (PLAYER_BULLET_VISUAL_MUL / PLAYER_BULLET_COLLISION_MUL);
}

constexpr int   RICH_BULLET_GLOW_ALPHA        = 110;
constexpr int   RICH_ENEMY_GLOW_ALPHA         = 90;
constexpr int   RICH_PLAYER_HALO_ALPHA        = 100;
constexpr int   RICH_STAR_COUNT               = 80;
constexpr int   RICH_GRID_SCROLL_SPEED        = 2;

// ============================================================
// [VISUAL_STYLE] キャラの基本シルエット
// Primitive  : 球・立方体のみ（最軽量）
// Silhouette : コードで組んだメカ／人型など（画像不要）
// 重さが気になったら Primitive に戻してください。
// 各実装箇所には // [VISUAL_STYLE] というコメントが付いています。
// ============================================================
enum class VisualStyle
{
	Primitive,
	Silhouette
};
constexpr VisualStyle VISUAL_STYLE = VisualStyle::Silhouette;

inline bool UseSilhouetteStyle()
{
	return VISUAL_STYLE == VisualStyle::Silhouette;
}

// ============================================================
// [VISUAL_THEME] 世界観テーマ
// Default : メカ＋SF（SilhouetteDraw のメカ／汎用シルエット）
// Touhou  : 巫女ビルボード・妖怪シルエット・星／札弾・雲パララックス
// 戻すときは VisualTheme::Default に変更。
// 各実装箇所には // [VISUAL_THEME] というコメントが付いています。
// ============================================================
enum class VisualTheme
{
	Default,
	Touhou
};
constexpr VisualTheme VISUAL_THEME = VisualTheme::Default;

inline bool UseTouhouTheme()
{
	return VISUAL_THEME == VisualTheme::Touhou;
}

// スプライト優先（東方テーマより先に判定する側で使用）
inline bool PreferGameSprites()
{
	return USE_GAME_SPRITES;
}

// 任意: assets/touhou/miko.png があれば優先（無ければコード生成テクスチャ）
constexpr const char* TOUHOU_MIKO_ASSET_PATH = "assets/touhou/miko.png";

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

inline const char* GetDifficultyDisplayName(Difficulty d)
{
	switch (d)
	{
	case Difficulty::Easy:    return "EASY";
	case Difficulty::Hard:    return "HARD";
	case Difficulty::Lunatic: return "LUNATIC";
	default:                  return "NORMAL";
	}
}

inline const BossRushDifficultyTune& GetBossRushTune(Difficulty d)
{
	int idx = (int)d;
	if (idx < 0 || idx >= DifficultyCount()) idx = 1;
	return BOSS_RUSH_TUNE[idx];
}

inline int ClampBossRushStartPhase(int phase)
{
	if (phase < 0) return 0;
	if (phase >= BOSS_RUSH_PHASE_COUNT) return BOSS_RUSH_PHASE_COUNT - 1;
	return phase;
}

inline float GetBossCollisionRadius(int phase)
{
	return BOSS_PHASE_COLLISION_RADIUS[ClampBossRushStartPhase(phase)];
}

inline const char* GetBossRushPhaseStartLabel(int phase)
{
	static const char* labels[BOSS_RUSH_PHASE_COUNT] = {
		"第１形態から",
		"第２形態から",
		"第３形態から",
		"第４形態から"
	};
	return labels[ClampBossRushStartPhase(phase)];
}

inline int GetBossRushPhaseHp(Difficulty d, int phase, bool practiceMode)
{
	phase = ClampBossRushStartPhase(phase);

	float hp = (float)BOSS_RUSH_PHASE_HP_BASE[phase] * GetBossRushTune(d).hpMul;
	if (practiceMode)
		hp *= 0.55f;
	int out = (int)(hp + 0.5f);
	if (out < 40) out = 40;
	return out;
}

inline int GetBossRushSpellBreakFrames(Difficulty d)
{
	switch (d)
	{
	case Difficulty::Easy:    return 195;
	case Difficulty::Lunatic: return 120;
	case Difficulty::Hard:    return 135;
	default:                  return SPELL_BREAK_FRAMES;
	}
}

inline int BossRushScaleBulletCount(int base, Difficulty d)
{
	int n = (int)((float)base * GetBossRushTune(d).bulletCountMul + 0.5f);
	if (n < 6) n = 6;
	return n;
}

inline int BossRushScaleFireInterval(int base, Difficulty d)
{
	int bonus = GetBossRushTune(d).fireIntervalBonus;
	int v = base + bonus * 4;
	if (v < 2) v = 2;
	return (int)((float)v * GetBossRushTune(d).danmakuIntervalMul + 0.5f);
}

// ボス／スペルカード表示名（面・フェーズ）
inline const char* GetBossDisplayName(int stageChapter)
{
	(void)stageChapter;
	if (BOSS_RUSH_MODE)
		return "究極弾幕機　デスティニーコア";
	if (stageChapter >= 1)
		return "第２面ボス　紅魔の残像";
	return "第１面ボス　弾幕の支配者";
}

inline const char* GetSpellCardName(int stageChapter, int phase)
{
	(void)stageChapter;
	if (phase < 0) phase = 0;
	if (BOSS_RUSH_MODE)
	{
		static const char* rush[BOSS_RUSH_PHASE_COUNT] = {
			"紅蓮弾符「業火の嵐」",
			"旋風符「螺旋の調べ」",
			"禁忌符「終末の弾幕」",
			"絶望符「弾幕覇王拳」"
		};
		if (phase >= BOSS_RUSH_PHASE_COUNT) phase = BOSS_RUSH_PHASE_COUNT - 1;
		return rush[phase];
	}
	if (phase > 2) phase = 2;
	static const char* ch0[] = {
		"紅蓮弾符「業火の嵐」",
		"旋風符「螺旋の調べ」",
		"禁忌符「終末の弾幕」"
	};
	static const char* ch1[] = {
		"月影符「冷凍の湖面」",
		"雷撃符「転送の落雷」",
		"夢幻符「弾幕覇王拳」"
	};
	return (stageChapter >= 1) ? ch1[phase] : ch0[phase];
}

