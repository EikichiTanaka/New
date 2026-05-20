#include "Scene/GameScene.h"
#include "Manager/SceneManager.h"
#include "Common/GameData.h"
#include "Common/KeyHelper.h"
#include "DxLib.h"

void GameScene::Init()
{
	m_Play.Init(g_GameData.difficulty);
	// 難易度画面で押していた Enter などを引き継がない
	KeyHelper::SyncCurrentKeys();
}

void GameScene::Update()
{
	const bool launch = KeyHelper::IsLaunchTrigger();
	const bool pause = KeyHelper::IsTrigger(KEY_INPUT_P);

	m_Play.Update(launch, pause);

	if (m_Play.IsFinished())
	{
		SceneManager::GetInstance().RequestChangeScene(SceneID::Result);
	}
}

void GameScene::Draw()
{
	SetBackgroundColor(5, 8, 14);
	m_Play.Draw();
}

void GameScene::Final()
{
}
