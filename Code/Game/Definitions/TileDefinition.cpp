#pragma once
#include "Engine\Core\ErrorWarningAssert.hpp"
#include "Engine\Core\XMLUtilities.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Game\GameCommon.hpp"
#include "Game\Definitions\TileDefinition.hpp"

std::map< std::string, TileDefinition* > TileDefinition:: s_tileDefinitions;

TileDefinition::TileDefinition( const tinyxml2::XMLElement& element )
{	
	m_name = ParseXmlAttribute(element, "name", m_name);
	m_baseSpriteCoords = ParseXmlAttribute(element, "baseSpriteCoords", m_baseSpriteCoords);
	m_baseSpriteTint = ParseXmlAttribute(element, "baseSpriteTint", m_baseSpriteTint);	

	m_allowsWalking = ParseXmlAttribute(element, "allowsWalking", m_allowsWalking);

	//load spritesheet name and definition
	std::string defaultSpriteSheetName = "Terrain_8x8.png";
	defaultSpriteSheetName = ParseXmlAttribute(element, "spriteSheetName", defaultSpriteSheetName);
	m_spriteSheetDimensions = ParseXmlAttribute(element, "spriteSheetDimensions", m_spriteSheetDimensions);

	if (!IsStringNullOrEmpty(defaultSpriteSheetName))
	{
		Renderer* theRenderer = Renderer::GetInstance();

		m_spriteSheet = new SpriteSheet(*theRenderer->CreateOrGetTexture(Stringf("Data/Images/%s", defaultSpriteSheetName.c_str())), m_spriteSheetDimensions.x, m_spriteSheetDimensions.y);
	}

	m_baseSpriteUVCoords = m_spriteSheet->GetTexCoordsForSpriteCoords(m_baseSpriteCoords);
}

void TileDefinition::Initialize(const std::string& filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for(const tinyxml2::XMLElement* definitionNode = pRoot->FirstChildElement(); definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		TileDefinition* newDef = new TileDefinition(*definitionNode);		

		s_tileDefinitions.insert(std::pair<std::string, TileDefinition*>(std::string(newDef->m_name), newDef));
	}	

	//debugger notification
	DebuggerPrintf("Loaded tile definitions!!!");
}
