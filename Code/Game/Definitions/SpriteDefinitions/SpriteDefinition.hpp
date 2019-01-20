#pragma once
#include <string>
#include "Engine\Math\AABB2.hpp"
#include "Engine\Math\Vector2.hpp"
#include <map>
#include <vector>
#include "Engine/Core/XMLUtilities.hpp"
#include "Engine/ThirdParty/tinyxml2/tinyxml2.h"


class SpriteDefinition
{
public:
	explicit SpriteDefinition( const tinyxml2::XMLElement& element);
	static void Initialize(const std::string& filePath);

public:
	std::string m_id;
	std::string m_diffuseSource; //image
	int m_pixelsPerUnit;
	std::string m_uvLayoutType;
	AABB2 m_uvs;
	Vector2 m_pivot;

	//previously used a static list in sd1 (adventure)
	static std::map<std::string, SpriteDefinition*> s_spriteDefinitions;	
};

SpriteDefinition* GetSpriteDefinitionById(std::string id);


