#pragma once
#include "Game/GameCommon.hpp"

constexpr float HEX_RADIUS = 0.5f;

class Tile;
class Unit;
class TileHeatMap;
class Player;

struct TileDefinition
{
	char m_symbol = ' ';
	std::string m_name = " ";
	bool m_isBlocked = false;

public:
	TileDefinition(XmlElement& element);
	static void InitializeTileDefs(char const* filePath);
	static void ClearDefinition();
	static TileDefinition* GetBySymbol(char symbol);
	static unsigned int GetType(TileDefinition* tileDef);
	static std::vector<TileDefinition*> s_tileDefs;
};


struct MapDefinition
{
	std::string m_name = " ";
	std::string m_overlayShader = " ";
	IntVec2 m_gridSize;
	Vec3 m_worldBoundsMin;
	Vec3 m_worldBoundsMax;

	std::vector<Tile*> m_tiles;
	std::vector<Unit*> m_units;

public:
	MapDefinition(XmlElement& element);
	void SetUpTiles(XmlElement* element);
	void SetUpUnits(XmlElement* element);
	static void InitializeMapDefs(char const* filePath);
	static void ClearDefinition();
	static MapDefinition* GetByName(std::string const& name);
	static std::vector<MapDefinition*> s_mapDefs;
};

class Tile
{
public:
	Tile(TileDefinition* tileDef);
	Tile(unsigned int type);
	unsigned int m_type;
	Unit* m_currentUnit = nullptr;
};

class Map
{
public:
	Map(Game* owner);
	virtual ~Map();
	
	void Startup();
	void Update(float deltaSeconds);
	void Render() const;
	void Shutdown();
	void DeleteGridData();

	void PopulateDistanceField(TileHeatMap& out_distanceField, IntVec2 startCoords, float maxCost) const;
	void SetHeatIfLessAndNonSolid(bool& isDirty, TileHeatMap& distField, IntVec2 coords, float value) const;

	void LoadMapDef(MapDefinition* mapDef);

	void ResetUnitsData();

	int GetTileIndex(int x, int y) const;
	IntVec2 GetTileCoord(int index) const;
	Vec2 GetTileWorldPosition(int x, int y) const;
	bool IsTileInWorldBounds(int x, int y) const;
	bool IsTileInInRangeCurrentHeatMap(int x, int y, int rangeMax, int rangeMin = 0) const;

	void SetCurrentCamera(Camera* camera);

	Tile* GetTile(int index) const;
	Tile* GetTile(IntVec2 coord) const;
	std::vector<IntVec2> GetShortestPathToCoord(IntVec2 start, IntVec2 end) const;
	
	void ShowUI(int row, int col);
	void CurrentPlayerInput();

	void Cancel();
	void EndTurn();
	void StartTurn();
	void HoldFire();
	void Attack(Tile* tile);
	void Move(IntVec2 coord);
	void Select(Tile* tile);

	void ReadDataUnit(Unit* unit, int lock_id = 0);

	bool IsYourTurn() const;
	bool IsReadyToBeginNetworkGame();
	bool IsEveryoneInTheGame();
	bool DidEveryoneQuit();
	void SetPlayerReady(int id);
	void SetPlayerConnected();
	void SetPlayerQuit(int id);

	int GetApplicationPlayerID() const;

	void GameEnd();
	
	void InitPlayers();
private:
	void RenderHover() const;
	void RenderCurrentUnitInformation() const;
	void RenderDebug() const;

	void UnitsUpdate(float deltaSeconds);
	void RaycastUpdate();
	void PlayersUpdate(float deltaSeconds);

public:

	Player* m_player1 = nullptr;
	Player* m_player2 = nullptr;

	TileHeatMap* m_heatMap = nullptr;
	MapDefinition* m_mapDef = nullptr;
	Game* m_game = nullptr;
	Camera* m_camera = nullptr;
	Vec3 m_bottomLeftPos = Vec3(-41, -10.5, 0);
	Vec2 m_backgroundSize = Vec2(82, 41);

	Material* m_material = nullptr;
	Shader* m_overlayShader = nullptr;

	std::vector<Unit> m_units;

	EulerAngles m_sunOrientation = EulerAngles(330, 25, 0);
	float m_sunIntensity = 1.f;
	float m_ambIntensity = 0.f;

	std::vector<Vertex_PCUTBN> m_BGvertexes;
	std::vector<unsigned int> m_BGindexes;
	VertexBuffer* m_BGVertexBuffer;
	IndexBuffer* m_BGIndexBuffer;

	std::vector<Vertex_PCU> m_GridVertexes;
	std::vector<unsigned int> m_GridIndexes;
	VertexBuffer* m_GridVertexBuffer;
	IndexBuffer* m_GridIndexBuffer;

	RaycastResult3D m_raycastVsPlane;
	bool m_debugDraw = false;

	IntVec2 m_currentFocusedCoord;
	Unit* m_currentSelectedUnit = nullptr;
	int m_currentPlayerIDTurn = 1;
	bool m_isPendingEndTurn = false;

	bool m_isGameEnded = false;
};
