#pragma once
#include <string>
#include "Engine\Math\IntVector2.hpp"
#include <map>
#include <vector>
#include "Engine/Core/XMLUtilities.hpp"
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"

struct IsoSpriteFacingStruct
{
	IntVector2 m_direction;
	std::string m_src;
	IntVector2 m_scale = IntVector2(1,1);
};

class IsoSpriteDefinition
{
public:
	explicit IsoSpriteDefinition( const tinyxml2::XMLElement& element);
	static void Initialize(const std::string& filePath);

	//sources should be unique
	std::string GetSourceByDirection(IntVector2 direction);
	IntVector2 GetScaleByDirection(IntVector2 direction);

public:
	std::string m_id;

	std::vector<IsoSpriteFacingStruct> m_facingStructs;	

	//previously used a static list in sd1 (adventure)
	static std::map<std::string, IsoSpriteDefinition*> s_isoSpriteDefinitions;
};


IsoSpriteDefinition* GetIsoSpriteDefinitionById(std::string id);