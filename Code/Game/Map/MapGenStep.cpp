#include "Game\Map\Map.hpp"
#include "Game\Map\MapGenStep.hpp"
#include "Game\Map\MapGenStep_FillAndEdge.hpp"
#include "Game\Map\MapGenStep_FromFile.hpp"
#include "Game\Map\MapGenStep_Mutate.hpp"
#include "Game\Map\MapGenStep_CellularAutomata.hpp"

MapGenStep::MapGenStep( const tinyxml2::XMLElement& generationStepElement)
{
	m_name = generationStepElement.Name();
	m_ifTags = ParseXmlAttribute( generationStepElement, "ifTags", m_ifTags );
	m_setTags = ParseXmlAttribute( generationStepElement, "setTags", m_ifTags );
}

MapGenStep* MapGenStep::CreateMapGenStep( const tinyxml2::XMLElement& genStepXmlElement )
{
	std::string elementName = genStepXmlElement.Name();	
	if(elementName == "FillAndEdge")
	{
		return new MapGenStep_FillAndEdge(genStepXmlElement);
	}
	if(elementName == "FromFile")
	{
		return new MapGenStep_FromFile(genStepXmlElement);
	}
	if(elementName == "Mutate")
	{
		return new MapGenStep_Mutate(genStepXmlElement);
	}
	if(elementName == "CellularAutomata")
	{
		return new MapGenStep_CellularAutomata(genStepXmlElement);
	}
	if(elementName == "RoomsAndPaths")
	{

	}

	return nullptr;
}