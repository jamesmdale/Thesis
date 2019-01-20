#pragma once
#include <string>
#include <map>
#include <vector>
#include "Engine/Core/XMLUtilities.hpp"
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"

struct IsoSpriteAnimFrameStruct
{
	std::string m_frameSource;
	float m_animationTimePercentage;
};

class IsoSpriteAnimDefinition
{
public:
	explicit IsoSpriteAnimDefinition( const tinyxml2::XMLElement& element);
	static void Initialize(const std::string& filePath);
	float GetDuration();

public:
	std::string m_id;
	bool m_doesLoop = false;
	std::vector<IsoSpriteAnimFrameStruct> m_frameStructs;

	//previously used a static list in sd1 (adventure)
	static std::map<std::string, IsoSpriteAnimDefinition*> s_isoSpriteAnimDefinitions;


};


