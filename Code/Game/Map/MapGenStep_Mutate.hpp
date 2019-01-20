#pragma once
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"
#include "Game/Map/MapGenStep.hpp"

class MapGenStep_Mutate : public MapGenStep
{
public:
	explicit MapGenStep_Mutate( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_Mutate() {};
	void Run( Map& map ); // "pure virtual", MUST be overridden by subclasses

private:
	std::string	m_name;
	float m_chanceToMutate = 0.f;
	TileDefinition*	m_mutateTileDef = nullptr;
};
