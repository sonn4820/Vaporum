#pragma once
#include "Game/GameCommon.hpp"

class Entity
{
public:
	Entity(Game* owner);
	virtual ~Entity();
	
	virtual void Update(float deltaSeconds) = 0;
	virtual void Render() const = 0;

	Mat44 GetModeMatrix() const;

public:
	Game* m_game = nullptr;
	Vec3 m_position;
	Vec3 m_velocity;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	Rgba8 m_color = Rgba8::COLOR_WHITE;
public:
};