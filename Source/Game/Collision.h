#pragma once
#include <cmath>

// ============================================================
// Collision.h
// 3D球体同士の判定など、ゲーム内で使用する衝突判定用関数
// ============================================================

namespace Collision
{
	// XZ平面のAABB粗判定（ボス等大きなターゲット用・球判定の前に使う）
	inline bool SphereXZBroadPhase(
		float ax, float az, float ar,
		float bx, float bz, float br)
	{
		const float sum = ar + br;
		const float dx = fabsf(ax - bx);
		const float dz = fabsf(az - bz);
		return dx <= sum && dz <= sum;
	}

	// 3D球と球の当たり判定
	inline bool SphereVsSphere(
		float ax, float ay, float az, float ar,
		float bx, float by, float bz, float br)
	{
		float dx = ax - bx;
		float dy = ay - by;
		float dz = az - bz;
		float distSq = dx * dx + dy * dy + dz * dz;
		float radSum = ar + br;
		return distSq <= radSum * radSum;
	}
}
