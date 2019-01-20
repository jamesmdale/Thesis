#pragma once
#include <map>
#include <string>
#include "Engine\Core\XMLUtilities.hpp"
#include "Engine\ThirdParty\tinyxml2\tinyxml2.h"
#include "Engine\Core\Rgba.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Engine\Renderer\SpriteSheet.hpp"

class TileDefinition
{
public:
	explicit TileDefinition( const tinyxml2::XMLElement& element );
	static void Initialize(const std::string& filePath);
public: 
	//list of tile definition member variables
	std::string m_name = "default";
	IntVector2 m_baseSpriteCoords;
	AABB2 m_baseSpriteUVCoords;
	Rgba m_baseSpriteTint = Rgba::WHITE;

	IntVector2 m_spriteSheetDimensions;
	SpriteSheet* m_spriteSheet = nullptr;

	bool m_allowsWalking = false;

	//static variables
	static std::map< std::string, TileDefinition* >	s_tileDefinitions;
};
