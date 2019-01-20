#include "Game/Map/MapGenStep_FromFile.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/Rgba.hpp"
#include <vector>
#include "Game/GameCommon.hpp"
#include "Game/Map/Map.hpp"

MapGenStep_FromFile::MapGenStep_FromFile( const tinyxml2::XMLElement& generationStepElement )
	: MapGenStep( generationStepElement )
{
	m_filename = ParseXmlAttribute(generationStepElement, "fileName", m_filename);

	Image newMapImage = Image("Data/Images/" + m_filename);

	m_imageBounds = newMapImage.GetDimensions();
	for(int xIndex = 0; xIndex < m_imageBounds.x; xIndex++)
	{
		for(int yIndex = 0; yIndex < m_imageBounds.y; yIndex++)
		{
			m_texelRgbas.push_back(newMapImage.GetTexel(xIndex, yIndex));
		}
	}
}

void MapGenStep_FromFile::Run( Map& map )
{
	int tileReplacementStartIndex = 0;
	int xDifference = 0;
	int yDifference = 0;

	int startingX = 0;
	int startingY = 0;

	if(map.GetDimensions().x > m_imageBounds.x)
	{
		xDifference = map.GetDimensions().x - m_imageBounds.x;
		startingX = GetRandomIntInRange(0, xDifference);
	}
	if(map.GetDimensions().y > m_imageBounds.y)
	{
		yDifference = map.GetDimensions().y - m_imageBounds.y;
		startingY = GetRandomIntInRange(0, yDifference);
	}	
	
	tileReplacementStartIndex = startingX * startingY;
	
	for(int tileIndex = tileReplacementStartIndex; tileIndex < (int)map.m_tiles.size(); tileIndex++)
	{
		TileDefinition* changeTileDef = nullptr;
		Rgba color = m_texelRgbas[tileIndex];		
		float colorAlpha = color.GetAlphaAsFloat();

		float randomChance = GetRandomFloatZeroToOne();

		if(randomChance < colorAlpha)
		{
			color.a = (unsigned char)255;
			if(color == Rgba::GREEN)
			{
				changeTileDef = TileDefinition::s_tileDefinitions["Grass"];
			}

			if(changeTileDef != nullptr)
			{
				map.m_tiles[tileIndex]->m_tileDefinition = changeTileDef;
			}
		}		
	}	
}
