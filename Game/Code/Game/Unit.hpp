#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/Model.hpp"

constexpr float ANIM_PLAYRATE = 3.0f;
constexpr float ANIM_DAMAGE_TIME = 2.f;

struct UnitDefinitions
{
	char m_symbol = ' ';
	std::string m_name = " ";
	std::string m_imageFilename = " ";
	std::string m_modelFilename = " ";
	std::string m_type = " ";

	Vec3 m_muzzlePosition;

	std::string m_hitEffectName = " ";
	std::string m_explosionEffectName = " ";
	std::string m_shotEffectName = " ";

	std::string m_hitAudioFilename = " ";
	std::string m_explosionAudioFilename = " ";
	std::string m_shotAudioFilename = " ";

	int m_groundAttackDamage = 0;
	int m_groundAttackRangeMin = 0;
	int m_groundAttackRangeMax = 0;
	int m_movementRange = 0;
	int m_defense = 0;
	int m_health = 0;


public:
	UnitDefinitions(XmlElement& element);

	static void InitializeUnitDefs(char const* filePath);
	static void ClearDefinition();
	static UnitDefinitions* GetByName(std::string const& name);
	static UnitDefinitions* GetBySymbol(char symbol);
	static std::vector<UnitDefinitions*> s_unitDefs;
};

class Unit
{
public:
	Unit(UnitDefinitions* def, int playerID, IntVec2 startcoord);
	virtual ~Unit();
	
	void Update(float deltaSeconds);
	void Render() const;
	void Shutdown();

	void LoadDataFromMap(Map* map);

	bool MoveUnit(IntVec2 newCoord);
	bool IsUnitOfThatCoordInAttackRange(IntVec2 coord) const;
	bool IsCoordInRangeOfMovement(IntVec2 coord) const;
	bool CanAttack() const;

	void TakeDamage(int damage);
	void Attack(Tile* tile);
	void Cancel();

	void SetData(Unit* unit);

	void Play_MoveAnimation(IntVec2 targetCoord);
	void RotateToHex(IntVec2 coord, float maxDeltaDegree);

	void Move_AnimationUpdate(float deltaSeconds);
	void TextDamage_AnimationUpdate(float deltaSeconds);

	void PlayEffect(std::string name, Vec3 position, EulerAngles orientation, Rgba8 color, Vec3 velocityDir, FloatRange speed = FloatRange(0.f, 0.f), EulerAngles angularRandom = EulerAngles(), FloatRange size = FloatRange(1.f, 1.f), Vec2 scaleByTime = Vec2(), float lifetime = 0.8f, IntRange number = IntRange(1, 1));
	
public:
	Map* m_map = nullptr;

	UnitDefinitions* m_unitDef = nullptr;
	Model* m_model = nullptr;

	IntVec2 m_currentCoord;
	int m_playerID = 0;

	bool m_isSelected = false;
	bool m_isMoved = false;
	bool m_isDoneForThisTurn = false;
	IntVec2 m_previousCoord;

	int m_health = 0;
	bool m_isDead = false;
	bool m_isDoneRotating = true;

	Sound m_attackSound;
	Sound m_hitSound;
	Sound m_explodeSound;
	Sound m_moveSound;

	// Moving Anim
	CatmullRomSpline2D m_MoveAnimSpline;
	float m_moveAnimTimer = 0.f;
	bool m_playingMoveAnim = false;
	EulerAngles m_previousOrientation;

	// Damage Anim
	Vec3 m_damangeBillboardPosition;
	Rgba8 m_damangeBillboardColor = Rgba8::COLOR_RED;
	float m_damangeBillboardScale = 1.f;
	float m_damageTimer = ANIM_DAMAGE_TIME;
	int m_damageTaken = 0;

	Unit* m_unitAttackedMe = nullptr;
public:
};