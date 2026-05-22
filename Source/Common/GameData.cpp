#include "Common/GameData.h"

GameData g_GameData;

char ComputeRankLetter(int score, bool isClear, int graze, int maxChain, Difficulty diff)
{
	int pts = 0;
	if (isClear) pts += 40;
	if (score >= 500000) pts += 30;
	else if (score >= 250000) pts += 20;
	else if (score >= 100000) pts += 10;
	if (graze >= 400) pts += 15;
	else if (graze >= 150) pts += 8;
	if (maxChain >= 10) pts += 15;
	else if (maxChain >= 6) pts += 8;
	if (diff == Difficulty::Lunatic) pts += 15;
	else if (diff == Difficulty::Hard) pts += 10;
	else if (diff == Difficulty::Normal) pts += 5;

	if (pts >= 85) return 'S';
	if (pts >= 65) return 'A';
	if (pts >= 45) return 'B';
	return 'C';
}
