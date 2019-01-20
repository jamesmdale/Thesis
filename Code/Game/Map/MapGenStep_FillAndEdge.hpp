#pragma once
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"
#include "Game/Map/MapGenStep.hpp"

class MapGenStep_FillAndEdge : public MapGenStep
{
public:
	explicit MapGenStep_FillAndEdge( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_FillAndEdge() {};
	void Run( Map& map ); // "pure virtual", MUST be overridden by subclasses

private:
	std::string		m_name;
	TileDefinition*		m_fillTileDef = nullptr;
	TileDefinition*		m_edgeTileDef = nullptr;
	int m_edgeThickness = 1;

};
