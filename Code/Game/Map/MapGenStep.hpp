#pragma once
#include <string>
#include "Engine\ThirdParty\tinyxml2/tinyxml2.h"
#include "Engine\Core\EngineCommon.hpp"
#include "Game\Map\Tile.hpp"
#include "Game\Definitions\TileDefinition.hpp"

class Map;
class MapGenStep
{
public:
	MapGenStep( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep() {};
	virtual void Run( Map& map ) = 0; // "pure virtual", MUST be overridden by subclasses

public:
	static MapGenStep* CreateMapGenStep( const tinyxml2::XMLElement& genStepXmlElement );

public:
	std::string	m_name;
	std::string m_ifTags = "";
	std::string m_setTags = "";
};
