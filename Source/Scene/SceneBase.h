#pragma once

// 全シーン共通のインターフェース
class SceneBase
{
public:
	virtual ~SceneBase() = default;
	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void Final() = 0;
};
