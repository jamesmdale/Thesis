#include "Game\Map\MapGenStep_Mutate.hpp"
#include "Engine\Utility\Tags.hpp"
#include "Game/Map/Map.hpp"

MapGenStep_Mutate::MapGenStep_Mutate( const tinyxml2::XMLElement& generationStepElement )
	: MapGenStep( generationStepElement )
{
	m_name = generationStepElement.Name();
	m_chanceToMutate = ParseXmlAttribute(generationStepElement, "chanceToMutate", m_chanceToMutate);

	std::string defaultMutateTile = "Grass";
	std::string mutateTileNameParsedString = ParseXmlAttribute(generationStepElement, "mutateTile", defaultMutateTile);
	m_mutateTileDef = TileDefinition::s_tileDefinitions[mutateTileNameParsedString];
}

void MapGenStep_Mutate::Run( Map& map )
{
	for(int tileIndex = 0; tileIndex < (int)map.m_tiles.size(); tileIndex++)
	{		
		float randomChance = GetRandomFloatZeroToOne();
		if(randomChance < m_chanceToMutate)
		{
			if(m_ifTags != "")
			{
				if(map.m_tiles[tileIndex]->m_tags.HasTags(m_ifTags))
				{
					map.m_tiles[tileIndex]->m_tileDefinition = m_mutateTileDef;
					map.m_tiles[tileIndex]->m_tags.SetOrRemoveTags(m_setTags);
				}						
			}
			else
			{
				map.m_tiles[tileIndex]->m_tileDefinition = m_mutateTileDef;
				map.m_tiles[tileIndex]->m_tags.SetOrRemoveTags(m_setTags);
			}
			
		}			
	}
}
