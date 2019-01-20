#pragma once
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"
#include "Game/Map/MapGenStep.hpp"
#include <vector>

class MapGenStep_FromFile : public MapGenStep
{
public:
	explicit MapGenStep_FromFile( const tinyxml2::XMLElement& genStepXmlElement );
	~MapGenStep_FromFile() {};
	void Run( Map& map ); // "pure virtual", MUST be overridden by subclasses

private:
	std::string	m_name;
	std::string m_filename = "invalid";
	std::vector<Rgba> m_texelRgbas;
	IntVector2 m_imageBounds;
};
