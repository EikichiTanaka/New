#pragma once

#include "GameConfig.h"

// [VISUAL_THEME] ���������o�i�ޏ��r���{�[�h�E���^�D�e�E�_�p�����b�N�X�j
namespace TouhouTheme
{
	void Init();
	void Final();
	bool IsReady();

	void DrawPlayerBillboard(float x, float y, float z, bool fever);
	void DrawCloudParallax(int frameCount);

	void DrawStarBullet(float x, float y, float z, float radius, unsigned int color);
	void DrawOfudaBullet(float x, float y, float z, float vx, float vz,
		float radius, unsigned int color);
}
