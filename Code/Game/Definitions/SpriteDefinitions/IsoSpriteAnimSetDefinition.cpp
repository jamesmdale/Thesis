#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimSetDefinition.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

std::map<std::string, IsoSpriteAnimSetDefinition*> IsoSpriteAnimSetDefinition::s_isoSpriteAnimSetDefinitions;
std::string animSetId;

IsoSpriteAnimSetDefinition::IsoSpriteAnimSetDefinition(const tinyxml2::XMLElement& element)
{
	
	m_id = ParseXmlAttribute(element, "id", m_id);	

	const tinyxml2::XMLElement* animNameElement = element.FirstChildElement("set");
	if(animNameElement)
	{
		for(const tinyxml2::XMLElement* definitionNode = animNameElement; definitionNode; definitionNode = definitionNode->NextSiblingElement())
		{
			IsoAnimationSetStruct animStruct;

			animStruct.m_animName = ParseXmlAttribute(*definitionNode, "id", animStruct.m_animName);

			const tinyxml2::XMLElement* animElement = definitionNode->FirstChildElement("anim");
			if(animElement)
			{	
				animStruct.m_isoAnimId = ParseXmlAttribute(*animElement, "id", animStruct.m_isoAnimId);		
			}

			m_isoAnimSetStructs.push_back(animStruct);
		}		
	}	
}

void IsoSpriteAnimSetDefinition::Initialize(const std::string & filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for(const tinyxml2::XMLElement* definitionNode = pRoot; definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		IsoSpriteAnimSetDefinition* newDef = new IsoSpriteAnimSetDefinition(*definitionNode);		
		s_isoSpriteAnimSetDefinitions.insert(std::pair<std::string, IsoSpriteAnimSetDefinition*>(newDef->m_id, newDef));
	}	
}

std::string IsoSpriteAnimSetDefinition::GetAnimationIdBySimplifiedName(std::string animName)
{
	for(int animStructIndex = 0; animStructIndex < (int)m_isoAnimSetStructs.size(); animStructIndex++)
	{		
		if(animName == m_isoAnimSetStructs[animStructIndex].m_animName)
		{
			return m_isoAnimSetStructs[animStructIndex].m_isoAnimId;
		}
	}
}
