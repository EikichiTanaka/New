#include "Manager/SceneManager.h"
#include "Scene/SceneBase.h"
#include "Scene/TitleScene.h"
#include "Scene/DifficultySelectScene.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"

static TitleScene s_Title;
static DifficultySelectScene s_Diff;
static GameScene s_Game;
static ResultScene s_Result;

SceneManager& SceneManager::GetInstance()
{
	static SceneManager inst;
	return inst;
}

void SceneManager::Init()
{
	m_CurrentId = SceneID::Title;
	m_Scene = &s_Title;
	m_Scene->Init();
}

void SceneManager::Update()
{
	if (m_ChangeReq)
	{
		ChangeScene(m_NextId);
		m_ChangeReq = false;
	}
	if (m_Scene) m_Scene->Update();
}

void SceneManager::Draw()
{
	if (m_Scene) m_Scene->Draw();
}

void SceneManager::Final()
{
	if (m_Scene) { m_Scene->Final(); m_Scene = nullptr; }
}

void SceneManager::RequestChangeScene(SceneID id)
{
	m_NextId = id;
	m_ChangeReq = true;
}

void SceneManager::ChangeScene(SceneID id)
{
	if (m_Scene) m_Scene->Final();

	switch (id)
	{
	case SceneID::Title:             m_Scene = &s_Title; break;
	case SceneID::DifficultySelect:  m_Scene = &s_Diff;  break;
	case SceneID::Game:              m_Scene = &s_Game;  break;
	case SceneID::Result:            m_Scene = &s_Result; break;
	}
	m_CurrentId = id;
	if (m_Scene) m_Scene->Init();
}
