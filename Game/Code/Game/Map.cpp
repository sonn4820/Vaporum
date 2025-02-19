#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Game/Unit.hpp"
#include "Game/Player.hpp"

std::vector<TileDefinition*> TileDefinition::s_tileDefs;
std::vector<MapDefinition*> MapDefinition::s_mapDefs;


TileDefinition::TileDefinition(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "Invalid")),
	m_symbol(ParseXmlAttribute(element, "symbol", ' ')),
	m_isBlocked(ParseXmlAttribute(element, "isBlocked", false))
{

}

void TileDefinition::InitializeTileDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");

	XmlElement* tileDefElement = rootElement->FirstChildElement();

	while (tileDefElement)
	{
		std::string name = tileDefElement->Name();
		GUARANTEE_OR_DIE(name == "TileDefinition", "Root child element is in the wrong format");
		TileDefinition* newTileDef = new TileDefinition(*tileDefElement);
		s_tileDefs.push_back(newTileDef);
		tileDefElement = tileDefElement->NextSiblingElement();
	}
}

void TileDefinition::ClearDefinition()
{
	for (size_t i = 0; i < s_tileDefs.size(); i++)
	{
		if (s_tileDefs[i] != nullptr)
		{
			delete s_tileDefs[i];
			s_tileDefs[i] = nullptr;
		}
	}
}


TileDefinition* TileDefinition::GetBySymbol(char symbol)
{
	for (size_t i = 0; i < s_tileDefs.size(); i++)
	{
		if (s_tileDefs[i]->m_symbol == symbol)
		{
			return s_tileDefs[i];
		}
	}
	return nullptr;
}

unsigned int TileDefinition::GetType(TileDefinition* tileDef)
{
	for (unsigned int i = 0; i < s_tileDefs.size(); i++)
	{
		if (s_tileDefs[i] == tileDef)
		{
			return i;
		}
	}
	ERROR_AND_DIE("Bad Index Type");
}

MapDefinition::MapDefinition(XmlElement& element)
	:m_name(ParseXmlAttribute(element, "name", "")),
	m_overlayShader(ParseXmlAttribute(element, "overlayShader", "")),
	m_gridSize(ParseXmlAttribute(element, "gridSize", IntVec2())),
	m_worldBoundsMin(ParseXmlAttribute(element, "worldBoundsMin", Vec3())),
	m_worldBoundsMax(ParseXmlAttribute(element, "worldBoundsMax", Vec3()))
{

}

void MapDefinition::SetUpTiles(XmlElement* element)
{
	std::string stringValue = element->GetText();
	Strings row = SplitStringOnDelimiter(stringValue, '\n', false);

	for (int rowIndex = (int)row.size() - 1; rowIndex >= 0; rowIndex--)
	{
		Strings column = SplitStringOnDelimiter(row[rowIndex], ' ', false);
		for (int columnIndex = 0; columnIndex < column.size(); columnIndex++)
		{
			TileDefinition* tileDef = TileDefinition::GetBySymbol(*column[columnIndex].data());
			Tile* newTile = new Tile(tileDef);
			m_tiles.push_back(newTile);
		}
	}
}

void MapDefinition::SetUpUnits(XmlElement* element)
{
	int id = ParseXmlAttribute(*element, "player", 0);

	std::string stringValue = element->GetText();
	Strings row = SplitStringOnDelimiter(stringValue, '\n', false);

	for (int rowIndex = (int)row.size() - 1; rowIndex >= 0; rowIndex--)
	{
		Strings column = SplitStringOnDelimiter(row[rowIndex], ' ', false);
		for (int columnIndex = 0; columnIndex < column.size(); columnIndex++)
		{
			UnitDefinitions* unitDef = UnitDefinitions::GetBySymbol(*column[columnIndex].data());
			if (unitDef)
			{
				Unit* newUnit = new Unit(unitDef, id, IntVec2(columnIndex, (int)row.size() - 1 - rowIndex));
				m_units.push_back(newUnit);
			}
		}
	}
}

void MapDefinition::InitializeMapDefs(char const* filePath)
{
	XmlDocument file;
	XmlError result = file.LoadFile(filePath);
	GUARANTEE_OR_DIE(result == tinyxml2::XML_SUCCESS, "FILE IS NOT LOADED");

	XmlElement* rootElement = file.RootElement();
	GUARANTEE_OR_DIE(rootElement, "Root Element is null");


	XmlElement* mapDefElement = rootElement->FirstChildElement();

	while (mapDefElement)
	{
		std::string mapDefName = mapDefElement->Name();
		GUARANTEE_OR_DIE(mapDefName == "MapDefinition", "Root child element is in the wrong format");
		MapDefinition* newMapDef = new MapDefinition(*mapDefElement);

		XmlElement* mapElements = mapDefElement->FirstChildElement();
		while (mapElements)
		{
			std::string name = mapElements->Name();
			if (name == "Tiles")
			{
				newMapDef->SetUpTiles(mapElements);
			}
			if (name == "Units")
			{
				newMapDef->SetUpUnits(mapElements);
			}
			mapElements = mapElements->NextSiblingElement();
		}

		s_mapDefs.push_back(newMapDef);
		mapDefElement = mapDefElement->NextSiblingElement();
	}
}

void MapDefinition::ClearDefinition()
{
	for (size_t i = 0; i < s_mapDefs.size(); i++)
	{
		if (s_mapDefs[i] != nullptr)
		{
			for (size_t t = 0; t < s_mapDefs[i]->m_tiles.size(); t++)
			{
				delete s_mapDefs[i]->m_tiles[t];
				s_mapDefs[i]->m_tiles[t] = nullptr;
			}

			for (size_t u = 0; u < s_mapDefs[i]->m_units.size(); u++)
			{
				delete s_mapDefs[i]->m_units[u];
				s_mapDefs[i]->m_units[u] = nullptr;
			}

			s_mapDefs[i]->m_tiles.clear();
			s_mapDefs[i]->m_units.clear();

			delete s_mapDefs[i];
			s_mapDefs[i] = nullptr;
		}
	}
}


MapDefinition* MapDefinition::GetByName(std::string const& name)
{
	for (size_t i = 0; i < s_mapDefs.size(); i++)
	{
		if (s_mapDefs[i]->m_name == name)
		{
			return s_mapDefs[i];
		}
	}
	return nullptr;
}


Tile::Tile(TileDefinition* tileDef)
{
	m_type = TileDefinition::GetType(tileDef);
}

Tile::Tile(unsigned int type)
	:m_type(type)
{

}


Map::Map(Game* owner)
	:m_game(owner)
{
}

Map::~Map()
{
	Shutdown();
}

void Map::Startup()
{
	m_material = new Material(g_theRenderer);
	m_material->LoadXML("Data/Materials/Moon.xml");

	AddVertsForQuad3D(m_BGvertexes, m_BGindexes,
		m_bottomLeftPos + Vec3(0, 0, 0.0f),
		m_bottomLeftPos + Vec3(m_backgroundSize.x, 0, 0.0f),
		m_bottomLeftPos + Vec3(0, m_backgroundSize.y, 0.0f),
		m_bottomLeftPos + Vec3(m_backgroundSize.x, m_backgroundSize.y, 0.0f),
		Rgba8::COLOR_WHITE
	);

	CalculateTangentSpaceBasisVectors(m_BGvertexes, m_BGindexes);

	m_BGVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCUTBN) * (unsigned int)m_BGvertexes.size());
	m_BGIndexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * (unsigned int)m_BGindexes.size());
	g_theRenderer->CopyCPUToGPU(m_BGvertexes.data(), (int)(m_BGvertexes.size() * sizeof(Vertex_PCUTBN)), m_BGVertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_BGindexes.data(), (int)(m_BGindexes.size() * sizeof(unsigned int)), m_BGIndexBuffer);
}

void Map::Update(float deltaSeconds)
{
	PlayersUpdate(deltaSeconds);

	RaycastUpdate();

	CurrentPlayerInput();

	UnitsUpdate(deltaSeconds);
}

void Map::Render() const
{
	g_theRenderer->BeginCamera(*m_camera);

	g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
	g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->SetLightConstants(m_sunOrientation.GetAsMatrix_IFwd_JLeft_KUp().GetIBasis3D(), m_sunIntensity, m_ambIntensity, m_camera->m_position, 0.f, 0.1f, 0.f, 1.f);
	g_theRenderer->SetModelConstants();

	// BACKGROUND RENDER
	g_theRenderer->BindShader(m_material->m_shader, m_material->m_vertexType);
	g_theRenderer->BindTexture(m_material->m_diffuseTexture, 0);
	g_theRenderer->BindTexture(m_material->m_normalTexure, 1);
	g_theRenderer->BindTexture(m_material->m_specGlossEmitTexure, 2);
	g_theRenderer->DrawIndexedBuffer(m_BGVertexBuffer, m_BGIndexBuffer, m_BGindexes.size(), 0, m_material->m_vertexType);

	// GRID RENDER
	g_theRenderer->BindShader(m_overlayShader);
	g_theRenderer->BindTexture(nullptr, 0);
	g_theRenderer->BindTexture(nullptr, 1);
	g_theRenderer->BindTexture(nullptr, 2);
	g_theRenderer->DrawIndexedBuffer(m_GridVertexBuffer, m_GridIndexBuffer, m_GridIndexes.size());


	g_theRenderer->SetLightConstants(LightConstants());
	g_theRenderer->SetModelConstants();
	g_theRenderer->BindShader(nullptr);
	g_theRenderer->BindTexture(nullptr, 0);
	g_theRenderer->BindTexture(nullptr, 1);
	g_theRenderer->BindTexture(nullptr, 2);
	g_theRenderer->SetDepthStencilMode(DepthMode::DISABLED);

	// MOUSE INTERACTION RENDER
	RenderCurrentUnitInformation();
	RenderHover();
	RenderDebug();

	// UNITS RENDER
	for (int i = 0; i < m_units.size(); i++)
	{
		m_units[i].Render();
	}

	g_theRenderer->EndCamera(*m_camera);
}

void Map::Shutdown()
{
	delete m_BGVertexBuffer;
	m_BGVertexBuffer = nullptr;

	delete m_BGIndexBuffer;
	m_BGIndexBuffer = nullptr;

	delete m_GridVertexBuffer;
	m_GridVertexBuffer = nullptr;

	delete m_GridIndexBuffer;
	m_GridIndexBuffer = nullptr;

	for (size_t i = 0; i < m_units.size(); i++)
	{
		m_units[i].Shutdown();
	}
	m_units.clear();
}

void Map::DeleteGridData()
{
	m_mapDef = nullptr;
	delete m_GridVertexBuffer;
	delete m_GridIndexBuffer;
	m_GridVertexes.clear();
	m_GridIndexes.clear();
	m_overlayShader = nullptr;
}

void Map::PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost) const
{
	TileHeatMap& distField = out_distanceField;
	distField.SetHeaEverywhere(maxCost);
	distField.SetHeatAt(startCoords, 0.f);

	float currentValue = 0.f;
	bool isDirty = true;
	while (isDirty)
	{
		isDirty = false;
		float nextValue = currentValue + 1.f;
		for (int tileY = 0; tileY < m_mapDef->m_gridSize.y; ++tileY)
		{
			for (int tileX = 0; tileX < m_mapDef->m_gridSize.x; ++tileX)
			{
				IntVec2 tileCoord = IntVec2(tileX, tileY);
				if (distField.GetHeatAt(tileCoord) == currentValue)
				{
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(0, 1), nextValue);
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(0, -1), nextValue);
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(1, 0), nextValue);
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(-1, 0), nextValue);
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(1, -1), nextValue);
					SetHeatIfLessAndNonSolid(isDirty, distField, tileCoord + IntVec2(-1, 1), nextValue);
				}
			}
		}
		currentValue += 1.f;
	}

	for (int tileY = 0; tileY < m_mapDef->m_gridSize.y; ++tileY)
	{
		for (int tileX = 0; tileX < m_mapDef->m_gridSize.x; ++tileX)
		{
			IntVec2 tileCoord = IntVec2(tileX, tileY);
			if (distField.GetHeatAt(tileCoord) == maxCost)
			{
				distField.SetHeatAt(tileCoord, -1.f);
			}
		}
	}
}

void Map::SetHeatIfLessAndNonSolid(bool& isDirty, TileHeatMap& distField, IntVec2 coords, float value) const
{
	if (!IsTileInWorldBounds(coords.x, coords.y))
	{
		return;
	}
	if (GetTile(GetTileIndex(coords.x, coords.y))->m_type == 0)
	{
		return;
	}
	if (distField.GetHeatAt(coords) < value)
	{
		return;
	}

	distField.SetHeatAt(coords, value);
	isDirty = true;
}

void Map::LoadMapDef(MapDefinition* mapDef)
{
	DeleteGridData();

	m_mapDef = mapDef;

	m_overlayShader = g_theRenderer->CreateShader(mapDef->m_overlayShader.c_str());

	for (int col = 0; col < mapDef->m_gridSize.y; col++)
	{
		for (int row = 0; row < mapDef->m_gridSize.x; row++)
		{
			if (!IsTileInWorldBounds(row, col))
			{
				continue;
			}

			Tile* tile = mapDef->m_tiles[GetTileIndex(row, col)];
			if (tile->m_type == 0)
			{
				AddVertForZHexagon(m_GridVertexes, m_GridIndexes, Vec3(GetTileWorldPosition(row, col), 0.0f), HEX_RADIUS, Rgba8::COLOR_BLACK);
			}
			else if (tile->m_type == 1)
			{
				AddVertForZHexagonOutline(m_GridVertexes, m_GridIndexes, Vec3(GetTileWorldPosition(row, col), 0.0f), HEX_RADIUS, Rgba8::COLOR_WHITE, 0.05f);
			}
		}
	}

	m_GridVertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU) * (unsigned int)m_GridVertexes.size());
	m_GridIndexBuffer = g_theRenderer->CreateIndexBuffer(sizeof(unsigned int) * (unsigned int)m_GridIndexes.size());
	g_theRenderer->CopyCPUToGPU(m_GridVertexes.data(), (int)(m_GridVertexes.size() * sizeof(Vertex_PCU)), m_GridVertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_GridIndexes.data(), (int)(m_GridIndexes.size() * sizeof(unsigned int)), m_GridIndexBuffer);


	for (int i = 0; i < m_mapDef->m_units.size(); i++)
	{
		m_units.push_back(*m_mapDef->m_units[i]);
	}

	for (int i = 0; i < m_units.size(); i++)
	{
		m_units[i].LoadDataFromMap(this);
		IntVec2 coord = m_units[i].m_currentCoord;
		Tile* tile = GetTile(GetTileIndex(coord.x, coord.y));
		tile->m_currentUnit = &m_units[i];
	}

	m_heatMap = new TileHeatMap(m_mapDef->m_gridSize);

	InitPlayers();
}

void Map::ResetUnitsData()
{
	for (int i = 0; i < m_mapDef->m_tiles.size(); i++)
	{
		m_mapDef->m_tiles[i]->m_currentUnit = nullptr;
	}
	for (int i = 0; i < m_mapDef->m_units.size(); i++)
	{
		m_units[i].SetData(m_mapDef->m_units[i]);
	}
}

int Map::GetTileIndex(int x, int y) const
{
	return y * m_mapDef->m_gridSize.x + x;
}

IntVec2 Map::GetTileCoord(int index) const
{
	int x = index % m_mapDef->m_gridSize.x;
	int y = index / m_mapDef->m_gridSize.x;

	return IntVec2(x, y);
}

Vec2 Map::GetTileWorldPosition(int x, int y) const
{
	return Vec2(x * 0.866f, 0.5f * x + y);
}

bool Map::IsTileInWorldBounds(int x, int y) const
{
	FloatRange xRange = FloatRange(m_mapDef->m_worldBoundsMin.x, m_mapDef->m_worldBoundsMax.x);
	FloatRange yRange = FloatRange(m_mapDef->m_worldBoundsMin.y, m_mapDef->m_worldBoundsMax.y);

	Vec2 worldPos = GetTileWorldPosition(x, y);

	return xRange.IsOnRange(worldPos.x) && yRange.IsOnRange(worldPos.y);
}

bool Map::IsTileInInRangeCurrentHeatMap(int x, int y, int rangeMax, int rangeMin) const
{
	int value = (int)m_heatMap->GetHeatAt(IntVec2(x, y));

	return value <= rangeMax && value >= rangeMin;
}

void Map::SetCurrentCamera(Camera* camera)
{
	m_camera = camera;
}

Tile* Map::GetTile(int index) const
{
	return m_mapDef->m_tiles[index];
}

Tile* Map::GetTile(IntVec2 coord) const
{
	return GetTile(GetTileIndex(coord.x, coord.y));
}

std::vector<IntVec2> Map::GetShortestPathToCoord(IntVec2 start, IntVec2 end) const
{
	std::vector<IntVec2> result;
	float startHeat = m_heatMap->GetHeatAt(start);
	float endHeat = m_heatMap->GetHeatAt(end);
	int range = (int)(endHeat - startHeat);

	IntVec2 evalutedCoord = end;
	result.push_back(evalutedCoord);

	for (size_t i = 0; i < range; i++)
	{
		IntVec2 neighborCoords[6];
		neighborCoords[0] = evalutedCoord + IntVec2(0, 1);
		neighborCoords[1] = evalutedCoord + IntVec2(0, -1);
		neighborCoords[2] = evalutedCoord + IntVec2(1, 0);
		neighborCoords[3] = evalutedCoord + IntVec2(-1, 0);
		neighborCoords[4] = evalutedCoord + IntVec2(1, -1);
		neighborCoords[5] = evalutedCoord + IntVec2(-1, 1);

		float neighborHeat[6];
		neighborHeat[0] = m_heatMap->GetHeatAt(neighborCoords[0]);
		neighborHeat[1] = m_heatMap->GetHeatAt(neighborCoords[1]);
		neighborHeat[2] = m_heatMap->GetHeatAt(neighborCoords[2]);
		neighborHeat[3] = m_heatMap->GetHeatAt(neighborCoords[3]);
		neighborHeat[4] = m_heatMap->GetHeatAt(neighborCoords[4]);
		neighborHeat[5] = m_heatMap->GetHeatAt(neighborCoords[5]);

		float lowestHeat = m_heatMap->GetHeatAt(evalutedCoord);
		IntVec2 lowestHeatCoords;
		for (int j = 0; j < 6; j++)
		{
			if (!IsTileInWorldBounds(neighborCoords[j].x, neighborCoords[j].y))
			{
				continue;
			}
			if (GetTile(neighborCoords[j])->m_type == 0 || m_heatMap->GetHeatAt(neighborCoords[j]) == -1.f)
			{
				neighborHeat[j] = 9999.f;
			}
			if (neighborHeat[j] < lowestHeat)
			{
				lowestHeatCoords = neighborCoords[j];
			}
		}
		result.push_back(lowestHeatCoords);
		evalutedCoord = lowestHeatCoords;
	}

	return result;
}

void Map::ShowUI(int row, int col)
{
	Tile* tile = GetTile(IntVec2(row, col));

	if (IsPointInsideZHexagon3D(m_raycastVsPlane.m_impactPos, Vec3(GetTileWorldPosition(row, col), 0.0f), HEX_RADIUS))
	{
		if (tile->m_currentUnit)
		{
			if (m_currentSelectedUnit)
			{
				ReadDataUnit(tile->m_currentUnit, m_currentSelectedUnit->m_playerID);
			}
			else
			{
				ReadDataUnit(tile->m_currentUnit);
			}

			if (tile->m_currentUnit->m_playerID == 1)
			{
				m_game->m_p1TankInfoPanel->SetActive(true);
			}
			else if (tile->m_currentUnit->m_playerID == 2)
			{
				m_game->m_p2TankInfoPanel->SetActive(true);
			}

			if (tile->m_currentUnit->m_playerID == m_currentPlayerIDTurn && !tile->m_currentUnit->m_isDoneForThisTurn)
			{
				m_game->m_LMBSprite->SetActive(true);
			}
		}
		else
		{
			if (!m_currentSelectedUnit)
			{
				m_game->m_RMBSprite->SetActive(false);
				m_game->m_LMBSprite->SetActive(false);

				m_game->m_YSprite->SetActive(true);
			}

			m_game->m_p1TankInfoPanel->SetActive(false);
			m_game->m_p2TankInfoPanel->SetActive(false);
		}

		if (m_currentSelectedUnit)
		{
			m_game->m_YSprite->SetActive(false);

			if (m_currentSelectedUnit->m_playerID == 1)
			{
				m_game->m_p1TankInfoPanel->SetActive(true);
			}
			else if (m_currentSelectedUnit->m_playerID == 2)
			{
				m_game->m_p2TankInfoPanel->SetActive(true);
			}

			m_game->m_LMBSprite->SetActive(true);
			m_game->m_RMBSprite->SetActive(true);

			if (!m_currentSelectedUnit->m_isMoved)
			{
				if (m_currentSelectedUnit->IsCoordInRangeOfMovement(IntVec2(row, col)))
				{
					m_game->m_LMBText->SetText("Move");

					if (tile->m_currentUnit == m_currentSelectedUnit)
					{
						m_game->m_LMBText->SetText("Stay");
					}
				}
				else
				{
					m_game->m_LMBSprite->SetActive(false);
				}
			}
			else
			{
				if (tile->m_currentUnit)
				{
					m_game->m_LMBSprite->SetActive(false);

					if (m_currentSelectedUnit->IsUnitOfThatCoordInAttackRange(tile->m_currentUnit->m_currentCoord))
					{
						m_game->m_LMBSprite->SetActive(true);
						m_game->m_LMBText->SetText("Fire");
					}

					if (IntVec2(row, col) == m_currentSelectedUnit->m_currentCoord)
					{
						m_game->m_LMBSprite->SetActive(true);
						m_game->m_LMBText->SetText("Hold Fire");
					}
				}
				else
				{
					m_game->m_LMBSprite->SetActive(false);
				}
			}

		}
	}

	if (!IsYourTurn())
	{
		m_game->m_RMBSprite->SetActive(false);
		m_game->m_LMBSprite->SetActive(false);
		m_game->m_YSprite->SetActive(false);
		m_game->m_LeftSprite->SetActive(false);
		m_game->m_RightSprite->SetActive(false);
	}
}

void Map::Cancel()
{
	if (!m_currentSelectedUnit)
	{
		return;
	}

	m_currentSelectedUnit->Cancel();

	m_currentSelectedUnit = nullptr;

	m_game->m_RMBSprite->SetActive(false);
	m_game->m_LMBSprite->SetActive(false);

	m_game->m_p1TankInfoPanel->SetActive(false);
	m_game->m_p2TankInfoPanel->SetActive(false);
}

void Map::CurrentPlayerInput()
{
	if (g_theInput->WasKeyJustPressed('B'))
	{
		m_debugDraw = !m_debugDraw;
	}

	if (m_game->m_dialoguePanel->IsActive())
	{
		if (m_isGameEnded)
		{
			if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER) || g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
			{
				GameEnd();
			}
		}
		else
		{
			if (IsYourTurn())
			{
				if (m_isPendingEndTurn)
				{
					if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
					{
						StartTurn();

						EventArgs args;
						args.SetValue("command", Stringf("\"StartTurn\""));
						g_theNetwork->RemoteCommand(args);
					}
					if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ESCAPE))
					{
						m_isPendingEndTurn = false;
						m_game->m_dialoguePanel->SetActive(false);
					}
				}
				else
				{
					if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE) || g_theInput->WasKeyJustPressed(KEYCODE_ENTER))
					{
						m_game->m_dialoguePanel->SetActive(false);
					}
				}

				return;
			}
		}
	}

	if (m_isGameEnded)
	{
		return;
	}

	if (IsYourTurn() && g_theInput->WasKeyJustPressed('Y'))
	{
		EndTurn();

		EventArgs args;
		args.SetValue("command", Stringf("\"EndTurn\""));
		g_theNetwork->RemoteCommand(args);
	}

	if (IsYourTurn() && g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		Cancel();

		EventArgs args;
		args.SetValue("command", Stringf("\"Cancel\""));
		g_theNetwork->RemoteCommand(args);
	}

	for (int col = 0; col < m_mapDef->m_gridSize.y; col++)
	{
		for (int row = 0; row < m_mapDef->m_gridSize.x; row++)
		{
			if (!IsTileInWorldBounds(row, col))
			{
				continue;
			}

			Tile* tile = GetTile(GetTileIndex(row, col));

			if (tile->m_type == 0)
			{
				continue;
			}

			if (!IsYourTurn())
			{
				break;
			}

			if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
			{
				if (IsPointInsideZHexagon3D(m_raycastVsPlane.m_impactPos, Vec3(GetTileWorldPosition(row, col), 0.0f), HEX_RADIUS))
				{
					if (!m_currentSelectedUnit && tile->m_currentUnit)
					{
						if (tile->m_currentUnit->m_isDoneForThisTurn)
						{
							break;
						}
						if (tile->m_currentUnit->m_playerID != m_currentPlayerIDTurn)
						{
							break;
						}

						Select(tile);

						EventArgs args;
						args.SetValue("command", Stringf("\"SelectFocusedUnit coords=%i,%i\"", row, col));
						g_theNetwork->RemoteCommand(args);
						break;
					}

					if (m_currentSelectedUnit)
					{
						if (m_currentSelectedUnit->m_playingMoveAnim || !m_currentSelectedUnit->m_isDoneRotating)
						{
							break;
						}
						if (m_currentSelectedUnit->m_isMoved)
						{
							if (tile->m_currentUnit == m_currentSelectedUnit)
							{
								HoldFire();

								EventArgs args;
								args.SetValue("command", Stringf("\"HoldFire\""));
								g_theNetwork->RemoteCommand(args);
								break;
							}

							if (tile->m_currentUnit)
							{
								if (m_currentSelectedUnit->IsUnitOfThatCoordInAttackRange(IntVec2(row, col)))
								{
									if (m_currentSelectedUnit->CanAttack())
									{
										Attack(tile);

										EventArgs args;
										args.SetValue("command", Stringf("\"Attack coords=%i,%i\"", row, col));
										g_theNetwork->RemoteCommand(args);
									}

									break;
								}
							}
							else
							{
								break;
							}
						}
						else
						{
							if (tile->m_currentUnit && tile->m_currentUnit != m_currentSelectedUnit)
							{
								break;
							}

							Move(IntVec2(row, col));

							EventArgs args;
							args.SetValue("command", Stringf("\"Move coords=%i,%i\"", row, col));
							g_theNetwork->RemoteCommand(args);
						}

						break;
					}
				}
			}
		}
	}
}


void Map::EndTurn()
{
	if (m_isPendingEndTurn)
	{
		return;
	}
	if (m_currentSelectedUnit)
	{
		return;
	}

	m_isPendingEndTurn = true;

	if (IsYourTurn())
	{
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("End Turn?");
		m_game->m_dialogueDetail->SetText("Press ENTER or Left Click to continue\nPress ESCAPE or Right Click to cancel");
	}

}

void Map::StartTurn()
{
	m_isPendingEndTurn = false;

	if (m_currentPlayerIDTurn == 1)
	{
		m_currentPlayerIDTurn = 2;
		if (!m_game->IsNetworkGame())
		{
			SetCurrentCamera(m_player2->GetCamera());
		}
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("Player 2's turn");
		m_game->m_dialogueDetail->SetText("Press ENTER or Click to continue");
	}
	else if (m_currentPlayerIDTurn == 2)
	{
		m_currentPlayerIDTurn = 1;
		if (!m_game->IsNetworkGame())
		{
			SetCurrentCamera(m_player1->GetCamera());
		}
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("Player 1's turn");
		m_game->m_dialogueDetail->SetText("Press ENTER or Click to continue");
	}

	for (auto& u : m_units)
	{
		u.m_isDoneForThisTurn = false;
		u.m_isMoved = false;
		u.m_isSelected = false;
	}
}

void Map::HoldFire()
{
	m_currentSelectedUnit->m_isDoneForThisTurn = true;
	m_currentSelectedUnit->m_isSelected = false;
	m_currentSelectedUnit = nullptr;
	m_game->m_YSprite->SetActive(true);
	m_game->m_RMBSprite->SetActive(false);
	m_game->m_LMBSprite->SetActive(false);
}

void Map::Attack(Tile* tile)
{
	m_currentSelectedUnit->Attack(tile);
	m_currentSelectedUnit->m_isDoneForThisTurn = true;
	m_currentSelectedUnit->m_isSelected = false;
	m_currentSelectedUnit = nullptr;
	m_game->m_YSprite->SetActive(true);
	m_game->m_RMBSprite->SetActive(false);
	m_game->m_LMBSprite->SetActive(false);
}

void Map::Move(IntVec2 coord)
{
	m_game->m_RMBText->SetText("Cancel");
	m_currentSelectedUnit->Play_MoveAnimation(coord);
	m_currentSelectedUnit->MoveUnit(coord);
	PopulateDistanceField(*m_heatMap, m_currentSelectedUnit->m_currentCoord, 9999.f);
}

void Map::Select(Tile* tile)
{
	m_currentSelectedUnit = tile->m_currentUnit;
	m_currentSelectedUnit->m_previousOrientation = m_currentSelectedUnit->m_model->m_orientation;
	m_currentSelectedUnit->m_isSelected = true;
	PopulateDistanceField(*m_heatMap, m_currentSelectedUnit->m_currentCoord, 9999.f);
}

void Map::ReadDataUnit(Unit* unit, int lock_id)
{
	if (!unit)
	{
		return;
	}

	if (unit->m_playerID == lock_id)
	{
		return;
	}

	UnitDefinitions* def = unit->m_unitDef;

	std::string name = def->m_name;

	Texture* image = nullptr;
	if (name == "Polar") image = m_game->m_PolarLogoImage;
	if (name == "Octopus") image = m_game->m_OctopusLogoImage;
	if (name == "Hadrian") image = m_game->m_HadrianLogoImage;
	if (name == "Grizzly") image = m_game->m_GrizzlyLogoImage;
	if (name == "Bison") image = m_game->m_BisonLogoImage;

	std::string stats = Stringf("%i\n%i\n%i-%i\n%i\n%i",
		def->m_groundAttackDamage,
		def->m_defense,
		def->m_groundAttackRangeMin,
		def->m_groundAttackRangeMax,
		def->m_movementRange,
		unit->m_health
	);

	if (lock_id == 0)
	{
		if (unit->m_playerID == 1)
		{
			m_game->m_p1TankInfoTitle->SetText(name);
			m_game->m_p1TankInfoSprite->SetTexture(image);
			m_game->m_p1TankInfoStats->SetText(stats);
		}
		if (unit->m_playerID == 2)
		{
			m_game->m_p2TankInfoTitle->SetText(name);
			m_game->m_p2TankInfoSprite->SetTexture(image);
			m_game->m_p2TankInfoStats->SetText(stats);
		}
	}

	if (lock_id == 1)
	{
		if (unit->m_playerID == 2)
		{
			m_game->m_p2TankInfoTitle->SetText(name);
			m_game->m_p2TankInfoSprite->SetTexture(image);
			m_game->m_p2TankInfoStats->SetText(stats);
		}
	}

	if (lock_id == 2)
	{
		if (unit->m_playerID == 1)
		{
			m_game->m_p1TankInfoTitle->SetText(name);
			m_game->m_p1TankInfoSprite->SetTexture(image);
			m_game->m_p1TankInfoStats->SetText(stats);
		}
	}
}

bool Map::IsYourTurn() const
{
	return (m_currentPlayerIDTurn == GetApplicationPlayerID()) || !m_game->IsNetworkGame();
}

bool Map::IsReadyToBeginNetworkGame()
{
	return m_player1->m_state == State::IS_CONNECTING && m_player2->m_state == State::IS_CONNECTING;
}

bool Map::IsEveryoneInTheGame()
{
	return m_player1->m_state == State::CONNECTED && m_player2->m_state == State::CONNECTED;
}

bool Map::DidEveryoneQuit()
{
	return m_player1->m_state == State::DISCONNECTED && m_player2->m_state == State::DISCONNECTED;
}

void Map::SetPlayerReady(int id)
{
	if (id == 1)
	{
		m_player1->m_state = State::IS_CONNECTING;
	}
	if (id == 2)
	{
		m_player2->m_state = State::IS_CONNECTING;
	}
	g_theDevConsole->AddLine(Rgba8::COLOR_DARK_YELLOW, Stringf("Player %i Ready", id));
}

void Map::SetPlayerConnected()
{
	m_player1->m_state = State::CONNECTED;
	m_player2->m_state = State::CONNECTED;
	g_theDevConsole->AddLine(Rgba8::COLOR_DARK_YELLOW, Stringf("Both Players Connected"));
}

void Map::SetPlayerQuit(int id)
{
	if (id == 1)
	{
		m_player1->m_state = State::DISCONNECTED;
	}
	if (id == 2)
	{
		m_player2->m_state = State::DISCONNECTED;
	}
	m_isGameEnded = true;
	g_theDevConsole->AddLine(Rgba8::COLOR_DARK_YELLOW, Stringf("Player %i Disconnected", id));
}

int Map::GetApplicationPlayerID() const
{
	if (g_theNetwork->IsServer()) return 1;
	if (g_theNetwork->IsClient()) return 2;
	return 0;
}

void Map::GameEnd()
{
	m_game->BackToMenu();
	m_game->m_dialoguePanel->SetActive(false);
	SetPlayerQuit(GetApplicationPlayerID());

	EventArgs args;
	args.SetValue("command", Stringf("\"PlayerQuit id=%i\"", GetApplicationPlayerID()));
	g_theNetwork->RemoteCommand(args);
}

void Map::InitPlayers()
{
	if (m_player1) delete m_player1;
	if (m_player2) delete m_player2;

	AABB3 worldBound = AABB3(m_mapDef->m_worldBoundsMin, m_mapDef->m_worldBoundsMax);
	Vec3 offsetY = m_mapDef->m_worldBoundsMax - m_mapDef->m_worldBoundsMin;
	offsetY.x = 0;
	offsetY.z = 0;

	m_player1 = new Player(m_game, worldBound.GetCenter() - offsetY * 0.8f);
	m_player2 = new Player(m_game, worldBound.GetCenter() - offsetY * 0.8f);

	if (m_game->IsNetworkGame())
	{
		if (g_theNetwork->IsServer())
		{
			SetCurrentCamera(m_player1->GetCamera());
		}
		if (g_theNetwork->IsClient())
		{
			SetCurrentCamera(m_player2->GetCamera());
		}
	}
	else
	{
		SetCurrentCamera(m_player1->GetCamera());
	}

	m_currentPlayerIDTurn = 1;
}

void Map::RenderHover() const
{
	Vec3 selectPos(GetTileWorldPosition(m_currentFocusedCoord.x, m_currentFocusedCoord.y), 0.0f);
	Tile* tile = GetTile(m_currentFocusedCoord);
	std::vector<Vertex_PCU> selectVertexes;
	std::vector<unsigned int> selectIndexes;

	if (m_currentSelectedUnit && !m_currentSelectedUnit->m_isMoved && IsTileInInRangeCurrentHeatMap(m_currentFocusedCoord.x, m_currentFocusedCoord.y, m_currentSelectedUnit->m_unitDef->m_movementRange))
	{
		std::vector<IntVec2> path = GetShortestPathToCoord(m_currentSelectedUnit->m_currentCoord, m_currentFocusedCoord);
		for (int i = 0; i < path.size(); i++)
		{
			Vec3 pathPos(GetTileWorldPosition(path[i].x, path[i].y), 0.0f);
			AddVertForZHexagon(selectVertexes, selectIndexes, pathPos, HEX_RADIUS, Rgba8(255, 255, 255, 150));
			AddVertForZHexagonOutline(selectVertexes, selectIndexes, pathPos, HEX_RADIUS, Rgba8::COLOR_YELLOW, 0.1f);
		}
	}
	if (tile->m_currentUnit && tile->m_currentUnit->m_playerID != m_currentPlayerIDTurn)
	{
		AddVertForZHexagonOutline(selectVertexes, selectIndexes, selectPos, HEX_RADIUS * 0.8f, Rgba8::COLOR_RED, 0.07f);
	}
	else
	{
		AddVertForZHexagonOutline(selectVertexes, selectIndexes, selectPos, HEX_RADIUS * 0.8f, Rgba8::COLOR_GREEN, 0.07f);
	}

	if (!IsYourTurn())
	{
		for (int col = 0; col < m_mapDef->m_gridSize.y; col++)
		{
			for (int row = 0; row < m_mapDef->m_gridSize.x; row++)
			{
				if (!IsTileInWorldBounds(row, col))
				{
					continue;
				}
				Vec3 yourSelectPos(GetTileWorldPosition(row, col), 0.0f);

				Tile* yourTile = GetTile(IntVec2(row, col));
				if (yourTile->m_type == 0)
				{
					continue;
				}
				if (IsPointInsideZHexagon3D(m_raycastVsPlane.m_impactPos, yourSelectPos, HEX_RADIUS))
				{
					if (yourTile->m_currentUnit && yourTile->m_currentUnit->m_playerID != GetApplicationPlayerID())
					{
						AddVertForZHexagonOutline(selectVertexes, selectIndexes, yourSelectPos, HEX_RADIUS * 0.8f, Rgba8::COLOR_DARK_RED, 0.07f);
					}
					else
					{
						AddVertForZHexagonOutline(selectVertexes, selectIndexes, yourSelectPos, HEX_RADIUS * 0.8f, Rgba8::COLOR_DARK_GREEN, 0.07f);
					}
				}
			}
		}
	}

	g_theRenderer->DrawIndexedBuffer(selectVertexes, selectIndexes);
}

void Map::RenderCurrentUnitInformation() const
{
	if (m_currentSelectedUnit)
	{
		Vec3 selectedPos(GetTileWorldPosition(m_currentSelectedUnit->m_currentCoord.x, m_currentSelectedUnit->m_currentCoord.y), 0.0f);

		std::vector<Vertex_PCU> selectedVertexes;
		std::vector<unsigned int> selectedIndexes;

		AddVertForZHexagonOutline(selectedVertexes, selectedIndexes, selectedPos, HEX_RADIUS * 0.9f, Rgba8::COLOR_BLUE, 0.1f);

		g_theRenderer->DrawIndexedBuffer(selectedVertexes, selectedIndexes);

		std::vector<Vertex_PCU> rangeVerts;
		std::vector<unsigned int>  rangeIndexes;

		for (int col = 0; col < m_mapDef->m_gridSize.y; col++)
		{
			for (int row = 0; row < m_mapDef->m_gridSize.x; row++)
			{
				if (!IsTileInWorldBounds(row, col))
				{
					continue;
				}
				if (m_currentSelectedUnit->m_isMoved)
				{
					if (m_currentSelectedUnit->IsUnitOfThatCoordInAttackRange(IntVec2(row, col)) && m_currentSelectedUnit->CanAttack())
					{
						Tile* tile = m_mapDef->m_tiles[GetTileIndex(row, col)];
						if (tile->m_type == 1)
						{
							Vec3 worldPos(GetTileWorldPosition(row, col), 0.0f);
							AddVertForZHexagon(rangeVerts, rangeIndexes, worldPos, HEX_RADIUS * 0.8f, Rgba8::COLOR_DARK_RED);
						}
					}
				}
				else
				{
					if (IsTileInInRangeCurrentHeatMap(row, col, m_currentSelectedUnit->m_unitDef->m_movementRange))
					{
						Tile* tile = m_mapDef->m_tiles[GetTileIndex(row, col)];
						if (tile->m_type == 1)
						{
							Vec3 worldPos(GetTileWorldPosition(row, col), 0.0f);
							AddVertForZHexagon(rangeVerts, rangeIndexes, worldPos, HEX_RADIUS, Rgba8(255, 255, 255, 100));
							AddVertForZHexagonOutline(rangeVerts, rangeIndexes, worldPos, HEX_RADIUS, Rgba8::COLOR_WHITE, 0.08f);
						}
					}
				}

			}
		}
		g_theRenderer->DrawIndexedBuffer(rangeVerts, rangeIndexes);
	}
}

void Map::RenderDebug() const
{
	if (m_debugDraw)
	{
		std::vector<Vertex_PCU> boundVertexes;
		AddVertsForWireframeAABB2D(boundVertexes, AABB2(m_mapDef->m_worldBoundsMin.x, m_mapDef->m_worldBoundsMin.y, m_mapDef->m_worldBoundsMax.x, m_mapDef->m_worldBoundsMax.y), Rgba8::COLOR_RED, 0.025f);
		AddVertsForSphere(boundVertexes, m_raycastVsPlane.m_impactPos, 0.025f, Rgba8::COLOR_VIOLET);
		g_theRenderer->DrawVertexArray(boundVertexes.size(), boundVertexes.data());

		std::vector<Vertex_PCU> outerGridVertexes;
		std::vector<unsigned int>  outerGridIndexes;
		for (int col = 0; col < m_mapDef->m_gridSize.y; col++)
		{
			for (int row = 0; row < m_mapDef->m_gridSize.x; row++)
			{
				if (!IsTileInWorldBounds(row, col))
				{
					Tile* tile = m_mapDef->m_tiles[GetTileIndex(row, col)];
					if (tile->m_type == 1)
					{
						AddVertForZHexagonOutline(outerGridVertexes, outerGridIndexes, Vec3(GetTileWorldPosition(row, col), 0.0f), HEX_RADIUS, Rgba8::COLOR_VIOLET, 0.025f);
					}
				}
			}
		}
		g_theRenderer->DrawIndexedBuffer(outerGridVertexes, outerGridIndexes);
	}
}

void Map::UnitsUpdate(float deltaSeconds)
{
	bool p1HasUnit = false;
	bool p2HasUnit = false;

	for (int i = 0; i < m_units.size(); i++)
	{
		m_units[i].Update(deltaSeconds);

		if (m_units[i].m_playerID == 1 && !m_units[i].m_isDead)
		{
			p1HasUnit = true;
		}

		if (m_units[i].m_playerID == 2 && !m_units[i].m_isDead)
		{
			p2HasUnit = true;
		}
	}

	if (m_currentSelectedUnit && !m_currentSelectedUnit->m_playingMoveAnim)
	{
		m_currentSelectedUnit->RotateToHex(m_currentFocusedCoord, deltaSeconds * 180.f);
	}

	if (!p1HasUnit)
	{
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("Player 2 Win");
		m_game->m_dialogueDetail->SetText("Press ENTER or Click to continue\nPress ESCAPE to cancel");
		m_isGameEnded = true;
	}
	if (!p2HasUnit)
	{
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("Player 1 Win");
		m_game->m_dialogueDetail->SetText("Press ENTER or Click to continue\nPress ESCAPE to cancel");
		m_isGameEnded = true;
	}
}

void Map::RaycastUpdate()
{
	if (m_isGameEnded)
	{
		return;
	}

	IntVec2 clientDim = Window::GetMainWindowInstance()->GetClientDimensions();
	Vec2 mousePos = g_theInput->GetCursorNormalizedPosition() * Vec2((float)clientDim.x, (float)clientDim.y);
	if (!m_game->IsNetworkGame())
	{
		m_raycastVsPlane = MouseRaycastVsPlane3D(m_camera, mousePos, Plane3());
	}
	else
	{
		if (g_theNetwork->IsServer())
		{
			m_raycastVsPlane = MouseRaycastVsPlane3D(m_player1->GetCamera(), mousePos, Plane3());
		}
		if (g_theNetwork->IsClient())
		{
			m_raycastVsPlane = MouseRaycastVsPlane3D(m_player2->GetCamera(), mousePos, Plane3());
		}
	}

	for (int col = 0; col < m_mapDef->m_gridSize.y; col++)
	{
		for (int row = 0; row < m_mapDef->m_gridSize.x; row++)
		{
			if (!IsTileInWorldBounds(row, col))
			{
				continue;
			}
			Vec3 selectPos(GetTileWorldPosition(row, col), 0.0f);

			Tile* tile = GetTile(IntVec2(row, col));
			if (tile->m_type == 0)
			{
				continue;
			}

			if (IsYourTurn())
			{
				if (IsPointInsideZHexagon3D(m_raycastVsPlane.m_impactPos, selectPos, HEX_RADIUS))
				{
					m_currentFocusedCoord = IntVec2(row, col);
					EventArgs args;
					args.SetValue("command", Stringf("\"SetFocusedHex coords=%i,%i\"", row, col));
					g_theNetwork->RemoteCommand(args);
				}
			}

			ShowUI(row, col);
		}
	}
}

void Map::PlayersUpdate(float deltaSeconds)
{
	if (m_game->IsNetworkGame() && !IsEveryoneInTheGame())
	{
		m_game->m_dialoguePanel->SetActive(true);
		m_game->m_dialogueTitle->SetText("Your opponent\nquit the game!");
		m_game->m_dialogueDetail->SetText("Click to go back to\nthe main menu");
		m_game->m_backToMenuFromGame->SetActive(true);
	}

	if (m_isGameEnded)
	{
		m_game->m_p1Panel->SetActive(false);
		m_game->m_p2Panel->SetActive(false);
		return;
	}

	if (m_currentPlayerIDTurn == 1)
	{
		m_game->m_p1Panel->SetActive(true);
		m_game->m_p2Panel->SetActive(false);
	}
	else if (m_currentPlayerIDTurn == 2)
	{
		m_game->m_p1Panel->SetActive(false);
		m_game->m_p2Panel->SetActive(true);
	}
	if (m_game->IsNetworkGame())
	{
		m_player1->Update(deltaSeconds);
		m_player2->Update(deltaSeconds);
	}
	else
	{
		if (m_currentPlayerIDTurn == 1)
		{
			m_player1->Update(deltaSeconds);
		}
		else if (m_currentPlayerIDTurn == 2)
		{
			m_player2->Update(deltaSeconds);
		}
	}

}
