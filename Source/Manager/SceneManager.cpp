// ============================================================
// SceneManager.cpp
// シーン管理クラスの実装
// ============================================================

#include "Manager/SceneManager.h"
#include "Scene/TitleScene.h"
#include "Scene/DifficultySelectScene.h"
#include "Scene/GameScene.h"
#include "Scene/ResultScene.h"
#include "Scene/TutorialScene.h"
#include "Scene/OptionsScene.h"
#include "Common/KeyHelper.h"

SceneManager::SceneManager()
	: m_pCurrentScene(nullptr)
{
}

SceneManager::~SceneManager()
{
	if (m_pCurrentScene != nullptr)
	{
		delete m_pCurrentScene;
		m_pCurrentScene = nullptr;
	}
}

// 初期シーン（タイトル画面）を生成して開始
void SceneManager::Init()
{
	ChangeScene(SceneType::Title);
}

// 現在のシーンを更新し、必要なら遷移を行う
bool SceneManager::Update()
{
	if (m_pCurrentScene == nullptr) return false;

	SceneType nextScene = m_pCurrentScene->Update();

	if (nextScene == SceneType::Exit)
	{
		return false;
	}

	if (nextScene != SceneType::None)
	{
		ChangeScene(nextScene);
	}

	return true;
}

void SceneManager::Draw()
{
	if (m_pCurrentScene != nullptr)
	{
		m_pCurrentScene->Draw();
	}
}

// 現在のシーンを破棄し、新しいシーンに切り替える
void SceneManager::ChangeScene(SceneType nextScene)
{
	if (m_pCurrentScene != nullptr)
	{
		delete m_pCurrentScene;
		m_pCurrentScene = nullptr;
	}

	switch (nextScene)
	{
	case SceneType::Title:
		m_pCurrentScene = new TitleScene();
		break;

	case SceneType::Options:
		m_pCurrentScene = new OptionsScene();
		break;

	case SceneType::DifficultySelect:
		m_pCurrentScene = new DifficultySelectScene();
		break;

	case SceneType::Game:
		m_pCurrentScene = new GameScene();
		break;

	case SceneType::Tutorial:
		m_pCurrentScene = new TutorialScene();
		break;

	case SceneType::Result:
		m_pCurrentScene = new ResultScene();
		break;

	default:
		return;
	}

	m_pCurrentScene->Init();
	KeyHelper::SyncCurrentKeys();
}