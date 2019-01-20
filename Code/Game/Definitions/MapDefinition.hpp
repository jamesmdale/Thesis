#pragma once
#include "Engine\ThirdParty\tinyxml2\tinyxml2.h"
#include <string>
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Math\IntRange.hpp"
#include "Game\Definitions\TileDefinition.hpp"
#include <vector>

class MapGenStep;
class MapDefinition
{
public:
	explicit MapDefinition( const tinyxml2::XMLElement& element );
	static void Initialize(const std::string& filePath);
	static MapDefinition* GetMapDefinitionByName(const std::string& mapName);

public:
	std::string m_name = "default";
	IntRange m_width;
	IntRange m_height;
	TileDefinition* m_defaultTile;
	float m_chanceToRun = 1.f;
	IntRange m_iterations = IntRange(1,1);

	std::vector<MapGenStep*> m_genSteps;	
	static std::map< std::string, MapDefinition* >	s_definitions;
	
};
