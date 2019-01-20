#include "Game/Map/MapGenStep_FillAndEdge.hpp"
#include "Game/Map/Map.hpp"

MapGenStep_FillAndEdge::MapGenStep_FillAndEdge( const tinyxml2::XMLElement& generationStepElement )
	: MapGenStep( generationStepElement )
{
	m_name = generationStepElement.Name();
	std::string defaultFillTileName = "Not Water";
	std::string fillTileNameParsedString = ParseXmlAttribute(generationStepElement, "fillTile", defaultFillTileName);
	m_fillTileDef = TileDefinition::s_tileDefinitions[fillTileNameParsedString];

	std::string defaultEdgeTileName = "Not Water";
	std::string edgeTileNameParsedString = ParseXmlAttribute(generationStepElement, "edgeTile", defaultEdgeTileName);
	m_edgeTileDef = TileDefinition::s_tileDefinitions[edgeTileNameParsedString];

	m_edgeThickness = ParseXmlAttribute( generationStepElement, "edgeThickness", m_edgeThickness );
}

void MapGenStep_FillAndEdge::Run( Map& map )
{
	//edge
	for(int tileIndex = 0; tileIndex < (int)map.m_tiles.size(); tileIndex++)
	{
		int xCoordinate = map.m_tiles[tileIndex]->m_tileCoords.x;
		int yCoordinate = map.m_tiles[tileIndex]->m_tileCoords.y;

		if(xCoordinate == 0 || xCoordinate == (map.m_dimensions.x - 1) || yCoordinate == 0 || yCoordinate == (map.m_dimensions.y - 1)) //if we are on the edge
		{
			map.m_tiles[tileIndex]->m_tileDefinition = m_edgeTileDef;		
		}
		else
		{
			map.m_tiles[tileIndex]->m_tileDefinition = m_fillTileDef;	
		}
	}	
}
