#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Math\AABB2.hpp"

//forward declarations
class Map;
class Agent;

enum ePointOfInterestType
{
	ARMORY_POI_TYPE,
	LUMBERYARD_POI_TYPE,
	MED_STATION_POI_TYPE,
	NUM_POI_TYPES
};

class PointOfInterest
{
public:
	PointOfInterest(ePointOfInterestType poiType, const IntVector2& startingCoordinate, const IntVector2& accessCoordinate, Map* mapReference);
	~PointOfInterest();

	void Update(float deltaSeconds);
	void Render();

	AABB2 GetWorldBounds();
	void TakeDamage(int damageAmount);

	IntVector2 GetCoordinateBoundsClosestToCoordinate(const IntVector2& coordinate);

public:
	int m_id = -1;
	int m_health = 100;

	uint16 m_agentCurrentlyServingIndex = UINT16_MAX;

	IntVector2 m_startingCoordinate;
	IntVector2 m_accessCoordinate;
	Vector2 m_accessPosition;
	ePointOfInterestType m_type;

	Stopwatch* m_refillTimer = new Stopwatch();
	Map* m_map = nullptr;

	//for now all poi are 2x2 blocks with an access point randomly touching one
};
