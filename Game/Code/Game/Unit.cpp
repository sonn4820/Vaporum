#include "Game/Unit.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

std::vector<UnitDefinitions*> UnitDefinitions::s_unitDefs;

UnitDefinitions::UnitDefinitions(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_symbol(ParseXmlAttribute(element, "symbol", ' ')),
	m_imageFilename(ParseXmlAttribute(element, "imageFilename", "")),
	m_modelFilename(ParseXmlAttribute(element, "modelFilename", "")),
	m_type(ParseXmlAttribute(element, "type", "")),
	m_groundAttackDamage(ParseXmlAttribute(element, "groundAttackDamage", 0)),
	m_groundAttackRangeMin(ParseXmlAttribute(element, "groundAttackRangeMin", 0)),
	m_groundAttackRangeMax(ParseXmlAttribute(element, "groundAttackRangeMax", 0)),
	m_movementRange(ParseXmlAttribute(element, "movementRange", 0)),
	m_defense(ParseXmlAttribute(element, "defense", 0)),
	m_health(ParseXmlAttribute(element, "health", 0)),
	m_muzzlePosition(ParseXmlAttribute(element, "muzzlePosition", Vec3::ZERO)),
	m_hitEffectName(ParseXmlAttribute(element, "hitEffectName", "")),
	m_explosionEffectName(ParseXmlAttribute(element, "explosionEffectName", "")),
	m_shotEffectName(ParseXmlAttribute(element, "shotEffectName", "")),
	m_hitAudioFilename(ParseXmlAttribute(element, "hitAudioFilename", "")),
	m_explosionAudioFilename(ParseXmlAttribute(element, "explosionAudioFilename", "")),
	m_shotAudioFilename(ParseXmlAttribute(element, "shotAudioFilename", ""))
{

}

void UnitDefinitions::InitializeUnitDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* unitDefElement = rootElement->FirstChildElement();

	while (unitDefElement)
	{
		std::string name = unitDefElement->Name();
		GUARANTEE_OR_DIE(name == "UnitDefinition", "Root child element is in the wrong format");
		UnitDefinitions* newUnitDef = new UnitDefinitions(*unitDefElement);
		s_unitDefs.push_back(newUnitDef);
		unitDefElement = unitDefElement->NextSiblingElement();
	}
}

void UnitDefinitions::ClearDefinition()
{
	for (size_t i = 0; i < s_unitDefs.size(); i++)
	{
		if (s_unitDefs[i] != nullptr)
		{
			delete s_unitDefs[i];
			s_unitDefs[i] = nullptr;
		}
	}
}

UnitDefinitions* UnitDefinitions::GetByName(std::string const& name)
{
	for (size_t i = 0; i < s_unitDefs.size(); i++)
	{
		if (s_unitDefs[i]->m_name == name)
		{
			return s_unitDefs[i];
		}
	}
	return nullptr;
}

UnitDefinitions* UnitDefinitions::GetBySymbol(char symbol)
{
	for (size_t i = 0; i < s_unitDefs.size(); i++)
	{
		if (s_unitDefs[i]->m_symbol == symbol)
		{
			return s_unitDefs[i];
		}
	}
	return nullptr;
}

Unit::Unit(UnitDefinitions* def, int playerID, IntVec2 startcoord)
	:m_unitDef(def), m_playerID(playerID), m_currentCoord(startcoord)
{

}

Unit::~Unit()
{

}

void Unit::Update(float deltaSeconds)
{
	TextDamage_AnimationUpdate(deltaSeconds);
	if (m_isDead) return;
	Move_AnimationUpdate(deltaSeconds);
}

void Unit::Render() const
{
	if (!m_isDead)
	{
		g_theRenderer->SetDepthStencilMode(DepthMode::ENABLED);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

		if (m_playerID == 1)
		{
			m_model->m_color = Rgba8(170, 170, 255);
		}
		else
		{
			m_model->m_color = Rgba8(255, 120, 120);
		}
		if (m_isSelected)
		{
			EulerAngles sunDir;
			sunDir.m_yawDegrees = 180;
			sunDir.m_pitchDegrees = 45;
			g_theRenderer->SetLightConstants(sunDir.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), 1.0, 0.0, m_map->m_camera->m_position, 0.f, 0.1f, 0.f, 1.f);
		}
		else
		{
			g_theRenderer->SetLightConstants(m_map->m_sunOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), m_map->m_sunIntensity, m_map->m_ambIntensity, m_map->m_camera->m_position, 0.f, 0.1f, 0.f, 1.f);
		}
		if (m_isDoneForThisTurn)
		{
			m_model->m_color *= 0.3f;
			m_model->m_color.a = 255;
		}

		m_model->Render();
	}

	// Damage Render
	if (m_damageTimer < ANIM_DAMAGE_TIME)
	{
		std::vector<Vertex_PCU> damageTextVerts;

		Mat44 textMatrix = GetBillboardMatrix(BilboardType::FULL_CAMERA_OPPOSING, m_map->m_camera->GetModelMatrix(), m_damangeBillboardPosition);

		g_UI->GetFont(0)->AddVertsForText3DAtOriginXForward(damageTextVerts, 0.5f * m_damangeBillboardScale, Stringf("%i", m_damageTaken), Rgba8::COLOR_WHITE, 0.5f);

		g_theRenderer->SetModelConstants(textMatrix, m_damangeBillboardColor);
		g_theRenderer->BindTexture(&g_UI->GetFont(0)->GetTexture());
		g_theRenderer->BindShader(nullptr);
		g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
		g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE);
		g_theRenderer->DrawVertexArray(damageTextVerts.size(), damageTextVerts.data());
	}
}

void Unit::Shutdown()
{
	delete m_model;
	m_model = nullptr;
}

void Unit::LoadDataFromMap(Map* map)
{
	m_map = map;

	m_health = m_unitDef->m_health;

	m_model = new Model(g_theApp->m_game);
	Vec2 worldPos = m_map->GetTileWorldPosition(m_currentCoord.x, m_currentCoord.y);
	m_model->m_position = Vec3(worldPos, 0);

	m_model->Load(m_unitDef->m_modelFilename);

	if (m_playerID == 2)
	{
		m_model->m_orientation = EulerAngles(180, 0, 0);
	}

	m_attackSound.id = g_theAudio->CreateOrGetSound(m_unitDef->m_shotAudioFilename);
	m_hitSound.id = g_theAudio->CreateOrGetSound(m_unitDef->m_hitAudioFilename);
	m_explodeSound.id = g_theAudio->CreateOrGetSound(m_unitDef->m_explosionAudioFilename);
	m_moveSound.id = g_theAudio->CreateOrGetSound("Data/Audio/Move.wav");
}

bool Unit::MoveUnit(IntVec2 newCoord)
{
	if (m_isDead) return false;

	if (!m_map->IsTileInInRangeCurrentHeatMap(newCoord.x, newCoord.y, m_unitDef->m_movementRange))
	{
		return false;
	}

	IntVec2 currentCoord = m_currentCoord;
	Tile* currentTile = m_map->GetTile(m_map->GetTileIndex(currentCoord.x, currentCoord.y));
	Tile* newTile = m_map->GetTile(m_map->GetTileIndex(newCoord.x, newCoord.y));

	if (newTile->m_currentUnit && newTile->m_currentUnit != this)
	{
		return false;
	}

	currentTile->m_currentUnit = nullptr;
	m_previousCoord = m_currentCoord;
	m_currentCoord = newCoord;
	newTile->m_currentUnit = this;
	m_isMoved = true;

	return true;
}

bool Unit::IsUnitOfThatCoordInAttackRange(IntVec2 coord) const
{
	if (m_isDead) return false;

	if (!m_map->GetTile(coord)->m_currentUnit)
	{
		return false;
	}
	if (m_map->GetTile(coord)->m_currentUnit->m_playerID == m_playerID)
	{
		return false;
	}
	if (!m_map->IsTileInInRangeCurrentHeatMap(coord.x, coord.y, m_unitDef->m_groundAttackRangeMax, m_unitDef->m_groundAttackRangeMin))
	{
		return false;
	}

	return true;
}

bool Unit::IsCoordInRangeOfMovement(IntVec2 coord) const
{
	if (!m_map->IsTileInInRangeCurrentHeatMap(coord.x, coord.y, m_unitDef->m_movementRange))
	{
		return false;
	}

	return true;
}

bool Unit::CanAttack() const
{
	if (m_isDead) return false;

	if (m_unitDef->m_type == "Tank")
	{
		return true;
	}
	if (m_unitDef->m_type == "Artillery")
	{
		if (m_previousCoord == m_currentCoord)
		{
			return true;
		}
	}

	return false;
}

void Unit::TakeDamage(int damage)
{
	if (m_isDead) return;

	m_damageTimer = 0.f;
	m_damangeBillboardPosition = m_model->m_position;
	m_damangeBillboardColor = Rgba8::COLOR_RED;
	m_damangeBillboardScale = 1.f;
	m_damageTaken = (int)(2 * damage / m_unitDef->m_defense);

	m_health -= m_damageTaken;

	if (m_health <= 0)
	{
		int angularDir = g_theRNG->RollRandomSign();
		PlayEffect(m_unitDef->m_hitEffectName, m_model->m_position, EulerAngles(), Rgba8(206, 74, 37),Vec3::ZERO, FloatRange(-0.05f, 0.05f), EulerAngles(5 * (float)angularDir, 0, 0), FloatRange(0.7f, 0.8f), Vec2(0.4f, 0.4f), 0.8f, IntRange(2, 3));
		PlayEffect(m_unitDef->m_explosionEffectName, m_model->m_position, EulerAngles(), Rgba8::COLOR_DARK_GRAY, Vec3::ZERO, FloatRange(-0.2f, 0.2f), EulerAngles(30 * (float)angularDir, 0, 0), FloatRange(1.4f, 1.7f), Vec2(0.7f, 0.7f), 1.2f, IntRange(4, 7));
		m_isDead = true;
		m_map->GetTile(m_currentCoord)->m_currentUnit = nullptr;
		m_explodeSound.playback = g_theAudio->StartSound(m_explodeSound.id);
	}
	else
	{
		Vec3 hitPosition = m_model->m_position;
		Vec3 hitDirection = (m_unitAttackedMe->m_model->m_position - m_model->m_position).GetNormalized();
		if (m_unitAttackedMe)
		{
			hitPosition += hitDirection * 0.2f;
		}
		PlayEffect(m_unitDef->m_hitEffectName, hitPosition, EulerAngles(), Rgba8(206, 74, 37), -hitDirection, FloatRange(0.1f, 0.5f), EulerAngles(10, 0, 0), FloatRange(0.8f, 1.2f), Vec2(0.2f, 0.2f), 0.8f, IntRange(3, 4));
		PlayEffect(m_unitDef->m_explosionEffectName, m_model->m_position, EulerAngles(), Rgba8::COLOR_DARK_X2_GRAY, Vec3::ZERO, FloatRange(0.0f, 0.0f), EulerAngles(0, 0, 0), FloatRange(0.6f, 0.8f), Vec2(2.5f, 2.5f), 0.8f, IntRange(1, 2));
		m_hitSound.playback = g_theAudio->StartSound(m_hitSound.id);
	}
}

void Unit::Attack(Tile* tile)
{
	if (m_isDead) return;

	if (!tile->m_currentUnit)
	{
		return;
	}

	Vec3 particlePos = m_unitDef->m_muzzlePosition.x * m_model->GetModeMatrix().GetIBasis3D()
		+ m_unitDef->m_muzzlePosition.y * m_model->GetModeMatrix().GetJBasis3D()
		+ m_unitDef->m_muzzlePosition.z * m_model->GetModeMatrix().GetKBasis3D();

	PlayEffect(m_unitDef->m_shotEffectName, m_model->m_position + particlePos, m_model->m_orientation, Rgba8::COLOR_ORANGE, Vec3::ZERO, FloatRange(-0.f, 0.f), EulerAngles(0, 0, 0), FloatRange(0.5f, 0.7f), Vec2(0.1f, 0.1f), 0.5f, IntRange(1, 1));
	tile->m_currentUnit->m_unitAttackedMe = this;
	tile->m_currentUnit->TakeDamage(m_unitDef->m_groundAttackDamage);

	m_attackSound.playback = g_theAudio->StartSound(m_attackSound.id);
}

void Unit::Cancel()
{
	if (m_isMoved)
	{
		Tile* currentTile = m_map->GetTile(m_map->GetTileIndex(m_currentCoord.x, m_currentCoord.y));
		Tile* previousTile = m_map->GetTile(m_map->GetTileIndex(m_previousCoord.x, m_previousCoord.y));

		currentTile->m_currentUnit = nullptr;
		m_currentCoord = m_previousCoord;
		previousTile->m_currentUnit = this;
		m_isMoved = false;
		m_playingMoveAnim = false;
	}

	m_isSelected = false;
	m_model->m_orientation = m_previousOrientation;
	g_theAudio->StopSound(m_moveSound.playback);
}

void Unit::SetData(Unit* unit)
{
	m_isSelected = false;
	m_isMoved = false;
	m_isDoneForThisTurn = false;
	m_isDead = false;

	m_unitDef = unit->m_unitDef;
	m_currentCoord = unit->m_currentCoord;
	m_previousCoord = m_currentCoord;
	m_playerID = unit->m_playerID;
	m_health = unit->m_unitDef->m_health;

	m_map->GetTile(m_currentCoord)->m_currentUnit = this;
}

void Unit::Play_MoveAnimation(IntVec2 targetCoord)
{
	if (m_isDead) return;

	if (!m_map->IsTileInInRangeCurrentHeatMap(targetCoord.x, targetCoord.y, m_unitDef->m_movementRange))
	{
		return;
	}

	std::vector<IntVec2> allLocs = m_map->GetShortestPathToCoord(m_currentCoord, targetCoord);
	std::vector<Vec2> allLocsFloat;
	if (!allLocs.empty())
	{
		for (int i = (int)allLocs.size() - 1; i >= 0; i--)
		{
			allLocsFloat.emplace_back(m_map->GetTileWorldPosition(allLocs[i].x, allLocs[i].y));
		}
	}

	m_MoveAnimSpline = CatmullRomSpline2D(allLocsFloat);
	m_playingMoveAnim = true;

	m_moveSound.playback = g_theAudio->StartSound(m_moveSound.id);
}

void Unit::RotateToHex(IntVec2 coord, float maxDeltaDegree)
{
	if (m_currentCoord == coord)
	{
		if (!m_isMoved)
		{
			m_model->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_model->m_orientation.m_yawDegrees, m_previousOrientation.m_yawDegrees, maxDeltaDegree);
		}

		m_isDoneRotating = true;
		return;
	}

	Vec3 direction = Vec3(m_map->GetTileWorldPosition(coord.x, coord.y), 0.f) - m_model->m_position;
	float angle = direction.GetAngleAboutZDegrees();

	if (m_isMoved)
	{
		if (IsUnitOfThatCoordInAttackRange(coord))
		{
			m_model->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_model->m_orientation.m_yawDegrees, angle, maxDeltaDegree);
		}
	}
	else
	{
		if (IsCoordInRangeOfMovement(coord))
		{
			m_model->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_model->m_orientation.m_yawDegrees, angle, maxDeltaDegree);
		}
	}
	float goalCompare = (angle < 0) ? angle + 360.f : angle;
	float currentCompare = (m_model->m_orientation.m_yawDegrees < 0) ? m_model->m_orientation.m_yawDegrees + 360.f : m_model->m_orientation.m_yawDegrees;
	if (goalCompare - currentCompare < 1.f)
	{
		m_isDoneRotating = true;
	}
	else
	{
		m_isDoneRotating = false;
	}
}

void Unit::Move_AnimationUpdate(float deltaSeconds)
{
	if (m_playingMoveAnim)
	{
		m_moveAnimTimer += deltaSeconds * ANIM_PLAYRATE;
		Vec3 nextPosition = Vec3(m_MoveAnimSpline.EvaluateAtParametric_Update(m_moveAnimTimer), 0);
		Vec3 direction = nextPosition - m_model->m_position;
		float angle = direction.GetAngleAboutZDegrees();

		m_model->m_orientation.m_yawDegrees = GetTurnedTowardDegrees(m_model->m_orientation.m_yawDegrees, angle, deltaSeconds * 180.f);
		m_model->m_position = nextPosition;

		if (m_MoveAnimSpline.m_hasFinished)
		{
			g_theAudio->StopSound(m_moveSound.playback);
			m_playingMoveAnim = false;
		}
	}
	else
	{
		m_moveAnimTimer = 0.f;
		m_model->m_position = Vec3(m_map->GetTileWorldPosition(m_currentCoord.x, m_currentCoord.y), 0);
	}

}

void Unit::TextDamage_AnimationUpdate(float deltaSeconds)
{
	m_damageTimer += deltaSeconds;

	if (m_damageTimer < ANIM_DAMAGE_TIME)
	{
		Mat44 textMatrix = GetBillboardMatrix(BilboardType::FULL_CAMERA_OPPOSING, m_map->m_camera->GetModelMatrix(), m_model->m_position);

		float rate = m_damageTimer / ANIM_DAMAGE_TIME;
		m_damangeBillboardPosition += textMatrix.GetKBasis3D() * 2.f * SmoothStop6(rate) * deltaSeconds;
		m_damangeBillboardScale += 2.5f * SmoothStop2(rate) * deltaSeconds;
		float alpha = NormalizeByte(m_damangeBillboardColor.a);
		alpha -= 20.f * SmoothStart2(rate) * deltaSeconds;
		alpha = Clamp(alpha, 0.f, 255.f);
		m_damangeBillboardColor.a = DenormalizeByte(alpha);
	}
}

void Unit::PlayEffect(std::string name, Vec3 position, EulerAngles orientation, Rgba8 color, Vec3 velocityDir, FloatRange speedRandom, EulerAngles angularRandom, FloatRange size, Vec2 scaleByTime, float lifetime, IntRange number)
{
	Emitter* emitter = g_theApp->m_game->m_particleSystem->GetEmitter(name);
	if (emitter)
	{
		emitter->m_lifeTime = lifetime;
		emitter->m_billboardType = BilboardType::FULL_CAMERA_FACING;
		emitter->SetParticlePositionAndOrientation(position, orientation);
		emitter->m_numParticleEachSpawn = number;
		emitter->m_particleSpeed = speedRandom;
		emitter->m_particleAngular = angularRandom;
		emitter->m_particleSize = size;
		emitter->m_particleColor = color;
		emitter->m_particleScale = scaleByTime;
		emitter->m_particleVelDirection = velocityDir;
		emitter->Activate();
	}
}
