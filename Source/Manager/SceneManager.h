#pragma once

enum class SceneID
{
	Title,
	DifficultySelect,
	Game,
	Result
};

class SceneManager
{
public:
	static SceneManager& GetInstance();
	void Init();
	void Update();
	void Draw();
	void Final();
	void RequestChangeScene(SceneID id);

private:
	SceneManager() = default;
	void ChangeScene(SceneID id);

	SceneID m_CurrentId = SceneID::Title;
	SceneID m_NextId = SceneID::Title;
	bool m_ChangeReq = false;
	class SceneBase* m_Scene = nullptr;
};
