#include "Game\Entities\PointOfInterest.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Map\Map.hpp"
#include "Game\Agents\Agent.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\EngineCommon.hpp"

//  =========================================================================================
PointOfInterest::PointOfInterest(ePointOfInterestType poiType, const IntVector2& startingCoordinate, const IntVector2& accessCoordinate, Map* mapReference)
{
	m_type = poiType;
	m_startingCoordinate = startingCoordinate;
	m_accessCoordinate = accessCoordinate;

	m_accessPosition = Vector2(0.5f, 0.5f) + Vector2(m_accessCoordinate);

	m_map = mapReference;

	m_id = m_map->m_pointsOfInterest.size();

	//start stopwatch
	m_refillTimer = new Stopwatch();
	m_refillTimer->SetTimer(g_baseResourceRefillTimePerSecond);
}

//  =========================================================================================
PointOfInterest::~PointOfInterest()
{
	m_map = nullptr;
}

//  =========================================================================================
void PointOfInterest::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

//  =========================================================================================
void PointOfInterest::Render()
{
	//unused;
	/*Renderer* theRenderer = Renderer::GetInstance();

	Rgba tint = Rgba::WHITE;
	switch (m_type)
	{
	case ARMORY_POI_TYPE:
		tint = ARMORY_TINT;
		break;
	case LUMBERYARD_POI_TYPE:
		tint = LUMBER_TINT;
		break;
	case MED_STATION_POI_TYPE:
		tint = MED_TINT;
		break;
	case WELL_POI_TYPE:
		tint = WELL_TINT;
		break;
	}

	AABB2 bounds;
	bounds.mins = m_map->GetWorldPositionOfMapCoordinate(m_startingCoordinate);
	bounds.maxs = m_map->GetWorldPositionOfMapCoordinate(m_startingCoordinate + IntVector2(2,2));

	theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
	theRenderer->DrawAABB(bounds, tint);
	theRenderer->SetShader(theRenderer->CreateOrGetShader("default"));

	theRenderer = nullptr;	*/
}

//  =========================================================================================
AABB2 PointOfInterest::GetWorldBounds()
{
	AABB2 bounds;
	bounds.mins = m_map->GetWorldPositionOfMapCoordinate(m_startingCoordinate);
	bounds.maxs = m_map->GetWorldPositionOfMapCoordinate(IntVector2(m_startingCoordinate.x + 2, m_startingCoordinate.y + 2));

	return bounds;
}

//  =========================================================================================
void PointOfInterest::TakeDamage(int damageAmount)
{
	m_health -= damageAmount;
	m_health = ClampInt(m_health, 0, 100);
}

//  =========================================================================================
IntVector2 PointOfInterest::GetCoordinateBoundsClosestToCoordinate(const IntVector2& coordinate)
{
	//set start to left wall
	IntVector2 closestCoordinate = IntVector2(m_startingCoordinate.x - 1, m_startingCoordinate.y + 1);
	int closestDistance = GetDistanceSquared(m_startingCoordinate, coordinate);
	
	//check lower bottom wall
	IntVector2 boundsCoordinate = IntVector2(m_startingCoordinate.x + 1, m_startingCoordinate.y - 1);
	int boundsDistance = GetDistanceSquared(boundsCoordinate, coordinate);
	if (GetDistanceSquared(boundsCoordinate, coordinate) < closestDistance && m_map->CheckIsCoordinateValid(boundsCoordinate))
	{
		closestCoordinate = boundsCoordinate;
		closestDistance = boundsDistance;
	}

	//check right wall
	boundsCoordinate = IntVector2(m_startingCoordinate.x + 3, m_startingCoordinate.y + 1);
	boundsDistance = GetDistanceSquared(boundsCoordinate, coordinate);
	if (GetDistanceSquared(boundsCoordinate, coordinate) < closestDistance && m_map->CheckIsCoordinateValid(boundsCoordinate))
	{
		closestCoordinate = boundsCoordinate;
		closestDistance = boundsDistance;
	}

	//check upper wall
	boundsCoordinate = IntVector2(m_startingCoordinate.x + 1, m_startingCoordinate.y + 3);
	boundsDistance = GetDistanceSquared(boundsCoordinate, coordinate);
	if (GetDistanceSquared(boundsCoordinate, coordinate) < closestDistance && m_map->CheckIsCoordinateValid(boundsCoordinate))
	{
		closestCoordinate = boundsCoordinate;
		closestDistance = boundsDistance;
	}

	return closestCoordinate;
}
