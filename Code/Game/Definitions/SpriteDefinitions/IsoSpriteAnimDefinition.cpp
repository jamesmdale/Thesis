#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimDefinition.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

std::map<std::string, IsoSpriteAnimDefinition*> IsoSpriteAnimDefinition::s_isoSpriteAnimDefinitions;

IsoSpriteAnimDefinition::IsoSpriteAnimDefinition(const tinyxml2::XMLElement& element)
{
	m_id = ParseXmlAttribute(element, "id", m_id);
	m_doesLoop = ParseXmlAttribute(element, "loop", m_doesLoop);

	const tinyxml2::XMLElement* frameElement = element.FirstChildElement("frame");
	if(frameElement)
	{
		for(const tinyxml2::XMLElement* definitionNode = frameElement; definitionNode; definitionNode = definitionNode->NextSiblingElement())
		{
			IsoSpriteAnimFrameStruct* frame = new IsoSpriteAnimFrameStruct();
			frame->m_frameSource = ParseXmlAttribute(*definitionNode, "src", frame->m_frameSource);		
			frame->m_animationTimePercentage = ParseXmlAttribute(*definitionNode, "duration", frame->m_animationTimePercentage);		

			m_frameStructs.push_back(*frame);
		}	
	}
}

void IsoSpriteAnimDefinition::Initialize(const std::string & filePath)
{
	tinyxml2::XMLDocument tileDefDoc;
	tileDefDoc.LoadFile(filePath.c_str());

	tinyxml2::XMLElement* pRoot = tileDefDoc.FirstChildElement();

	for(const tinyxml2::XMLElement* definitionNode = pRoot; definitionNode; definitionNode = definitionNode->NextSiblingElement())
	{
		IsoSpriteAnimDefinition* newDef = new IsoSpriteAnimDefinition(*definitionNode);		
		s_isoSpriteAnimDefinitions.insert(std::pair<std::string, IsoSpriteAnimDefinition*>(newDef->m_id, newDef));
	}	
}

float IsoSpriteAnimDefinition::GetDuration()
{
	float time = 0.0f;

	for(int spriteIndex = 0; spriteIndex < (int)m_frameStructs.size(); spriteIndex++)
	{
		time += m_frameStructs[spriteIndex].m_animationTimePercentage;
	}

	return time;
}