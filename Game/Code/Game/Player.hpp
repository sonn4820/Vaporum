#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

enum class State
{
	DISCONNECTED,
	IS_CONNECTING,
	CONNECTED,
	STATE_NUM
};

class Player: public Entity
{
public:
	Player(Game* owner, Vec3 position);
	virtual ~Player();
	
	virtual void Update(float deltaSeconds) override;
	virtual void Render() const override;

	Camera* GetCamera();
public:
	State m_state = State::DISCONNECTED;
private:
	Camera* m_playerCamera = nullptr;
	float m_movingSpeed = 10.f;
	float m_rotatingSpeed = 90.f;
private:
	void HandleInput(float deltaSeconds);
};