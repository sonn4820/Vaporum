#include "Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"

Player::Player(Game* owner, Vec3 position)
	:Entity(owner)
{
	m_playerCamera = new Camera();
	m_position = Vec3(position.x, position.y, 8.f);
	m_orientation = EulerAngles(90.f, 60.f, 0.f);
	m_playerCamera->SetPerspectiveView(Window::GetMainWindowInstance()->GetAspect(), 60.f, 0.01f, 100.f);
	m_playerCamera->SetRenderBasis(Vec3(0.f, 0.f, 1.f), Vec3(-1.f, 0.f, 0.f), Vec3(0.f, 1.f, 0.f));
	m_playerCamera->SetTransform(m_position, m_orientation);
}

Player::~Player()
{
}

void Player::Update(float deltaSeconds)
{
	HandleInput(deltaSeconds);
	m_playerCamera->SetTransform(m_position, m_orientation);
}

void Player::Render() const
{
}

Camera* Player::GetCamera()
{
	return m_playerCamera;
}

void Player::HandleInput(float deltaSeconds)
{
	Vec3 forwardDir = GetModeMatrix().GetIBasis3D().GetNormalized();
	Vec3 rightDir = GetModeMatrix().GetJBasis3D().GetNormalized();
	Vec3 upDir = GetModeMatrix().GetKBasis3D().GetNormalized();

	if (g_theInput->IsKeyDown('W'))
	{
		m_position += Vec3(forwardDir.x, forwardDir.y, 0).GetNormalized() * m_movingSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('S'))
	{
		m_position -= Vec3(forwardDir.x, forwardDir.y, 0).GetNormalized() * m_movingSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('A'))
	{
		m_position += Vec3(rightDir.x, rightDir.y, 0).GetNormalized() * m_movingSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('D'))
	{
		m_position -= Vec3(rightDir.x, rightDir.y, 0).GetNormalized() * m_movingSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('Q'))
	{
		m_position += Vec3(0,0,1).GetNormalized() * m_movingSpeed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('E'))
	{
		m_position -= Vec3(0, 0, 1).GetNormalized() * m_movingSpeed * deltaSeconds;
	}

	Vec3 min = m_game->m_map->m_mapDef->m_worldBoundsMin;
	Vec3 max = m_game->m_map->m_mapDef->m_worldBoundsMax;
	RaycastResult3D raycastToPlane = RaycastVsPlane3D(m_position, GetModeMatrix().GetIBasis3D(), FLT_MAX, Plane3());

	Vec3 onPlane = raycastToPlane.m_impactPos;
	onPlane.x = Clamp(onPlane.x, min.x, max.x);
	onPlane.y = Clamp(onPlane.y, min.y, max.y);

	m_position.z = Clamp(m_position.z, 1.f, max.z);

	m_position.x = onPlane.x - raycastToPlane.m_rayFwdNormal.x * raycastToPlane.m_impactDist;
	m_position.y = onPlane.y - raycastToPlane.m_rayFwdNormal.y * raycastToPlane.m_impactDist;
}