#pragma once
#include <string>
#include <map>
#include <vector>
#include "Engine/Core/XMLUtilities.hpp"
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"

struct IsoAnimationSetStruct 
{
	std::string m_animName;
	std::string m_isoAnimId;
};

class IsoSpriteAnimSetDefinition
{
public:
	explicit IsoSpriteAnimSetDefinition( const tinyxml2::XMLElement& element);
	static void Initialize(const std::string& filePath);
	std::string GetAnimationIdBySimplifiedName(std::string animName);

public:
	std::string m_id;

	std::vector<IsoAnimationSetStruct> m_isoAnimSetStructs;

	//previously used a static list in sd1 (adventure)
	static std::map<std::string, IsoSpriteAnimSetDefinition*> s_isoSpriteAnimSetDefinitions;
};



