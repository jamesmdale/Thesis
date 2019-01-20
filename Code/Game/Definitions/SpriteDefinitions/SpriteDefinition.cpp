#include "Game\Definitions\SpriteDefinitions\SpriteDefinition.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

std::map<std::string, SpriteDefinition*> SpriteDefinition::s_spriteDefinitions;

SpriteDefinition::SpriteDefinition(const tinyxml2::XMLElement& element)
{
	m_id = ParseXmlAttribute(element, "id", m_id);

	const tinyxml2::XMLElement* diffuseElement = element.FirstChildElement("diffuse");
	if(diffuseElement)
	{
		m_diffuseSource = ParseXmlAttribute(*diffuseElement, "src", m_diffuseSource);
	}

	const tinyxml2::XMLElement* ppuElement = element.FirstChildElement("ppu");
	if(ppuElement)
	{
		m_pixelsPerUnit = ParseXmlAttribute(*ppuElement, "count", m_pixelsPerUnit);
	}

	const tinyxml2::XMLElement* uvElement = element.FirstChildElement("uv");
	if(uvElement)
	{
		m_uvLayoutType = ParseXmlAttribute(*uvElement, "layout", m_uvLayoutType);
		m_uvs = ParseXmlAttribute(*uvElement, "uvs", m_uvs);
	}

	const tinyxml2::XMLElement* pivotElement = element.FirstChildElement("pivot");
	if(pivotElement)
	{
		m_pivot = ParseXmlAttribute(*pivotElement, "xy", m_pivot);
	}

}

void SpriteDefinition::Initialize(const std::string & filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for(const tinyxml2::XMLElement* definitionNode = pRoot; definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		SpriteDefinition* newDef = new SpriteDefinition(*definitionNode);		
		s_spriteDefinitions.insert(std::pair<std::string, SpriteDefinition*>(newDef->m_id, newDef));
	}	
}

SpriteDefinition* GetSpriteDefinitionById(std::string id)
{
	std::map<std::string, SpriteDefinition*>::iterator spriteDefIterator = SpriteDefinition::s_spriteDefinitions.find(id);
	if (spriteDefIterator != SpriteDefinition::s_spriteDefinitions.end())
	{
		return spriteDefIterator->second;
	}		

	return nullptr;
}
