#include "Game\Definitions\SpriteDefinitions\IsoSpriteDefinition.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

std::map<std::string, IsoSpriteDefinition*> IsoSpriteDefinition::s_isoSpriteDefinitions;

IsoSpriteDefinition::IsoSpriteDefinition(const tinyxml2::XMLElement& element)
{
	m_id = ParseXmlAttribute(element, "id", m_id);

	const tinyxml2::XMLElement* facingElement = element.FirstChildElement("facing");
	if(facingElement)
	{
		for(const tinyxml2::XMLElement* definitionNode = facingElement; definitionNode; definitionNode = definitionNode->NextSiblingElement())
		{
			IsoSpriteFacingStruct* facingStruct = new IsoSpriteFacingStruct();
			facingStruct->m_direction = ParseXmlAttribute(*definitionNode, "dir", facingStruct->m_direction);		
			facingStruct->m_src = ParseXmlAttribute(*definitionNode, "src", facingStruct->m_src);	
			facingStruct->m_scale = ParseXmlAttribute(*definitionNode, "scale", facingStruct->m_scale);

			m_facingStructs.push_back(*facingStruct);
		}		
	}
}

void IsoSpriteDefinition::Initialize(const std::string & filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for(const tinyxml2::XMLElement* definitionNode = pRoot; definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		IsoSpriteDefinition* newDef = new IsoSpriteDefinition(*definitionNode);		
		s_isoSpriteDefinitions.insert(std::pair<std::string, IsoSpriteDefinition*>(newDef->m_id, newDef));
	}	
}

std::string IsoSpriteDefinition::GetSourceByDirection(IntVector2 direction)
{
	for(int facingStructIndex = 0; facingStructIndex < (int)m_facingStructs.size(); facingStructIndex++)
	{
		if(m_facingStructs[facingStructIndex].m_direction == direction)
		{
			return m_facingStructs[facingStructIndex].m_src;
		}
	}

	return "";
}

IntVector2 IsoSpriteDefinition::GetScaleByDirection(IntVector2 direction)
{
	for(int facingStructIndex = 0; facingStructIndex < (int)m_facingStructs.size(); facingStructIndex++)
	{
		if(m_facingStructs[facingStructIndex].m_direction == direction)
		{
			return m_facingStructs[facingStructIndex].m_scale;
		}
	}

	return IntVector2(0,0);
}

IsoSpriteDefinition* GetIsoSpriteDefinitionById(std::string id)
{
	std::map<std::string, IsoSpriteDefinition*>::iterator spriteDefIterator = IsoSpriteDefinition::s_isoSpriteDefinitions.find(id);
	if (spriteDefIterator != IsoSpriteDefinition::s_isoSpriteDefinitions.end())
	{
		return spriteDefIterator->second;
	}		

	return nullptr;
}
