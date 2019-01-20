#pragma once
#include "Engine\ThirdParty\tinyxml2\tinyxml2.h"
#include "Game\Map\MapGenStep.hpp"
#include "Engine\Math\IntRange.hpp"
#include <vector>

class MapGenStep_CellularAutomata : public MapGenStep
{
public:
	explicit MapGenStep_CellularAutomata( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_CellularAutomata() {};
	void Run( Map& map ); // "pure virtual", MUST be overridden by subclasses

	std::vector<int> GetNeighboringTiles(int currentTileIndex, const Map& map);

private:
	std::string		m_name;
	TileDefinition* m_ifType = nullptr;
	TileDefinition* m_changeToType = nullptr;
	TileDefinition* m_ifNeighborType = nullptr;
	IntRange m_ifNeighborCount = IntRange(-1);
	float m_chanceToMutate = 1.f;
	float m_chanceToRun = 1.f;
	IntRange m_iterations = IntRange(1);

};
