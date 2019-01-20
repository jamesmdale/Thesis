#pragma once
#include "Engine\Time\Stopwatch.hpp"

class Map;
class EncounterState
{
public:
	EncounterState();
	~EncounterState();

	void Update(float deltaSeconds);

	//void ChangeThreatRatePerSecond();
	//void ChangeBombardmentRatePerSecond(); (eventually for all sides - north, south, east, west
	//void 

public:

	Map* m_mapReference = nullptr;
};

