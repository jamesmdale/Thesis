#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Utility\Tags.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Engine\Core\Widget.hpp"
#include "Game\Definitions\TileDefinition.hpp"
#include "Engine\Core\Rgba.hpp"
#include <vector>


class Tile //:public Widget
{
public:

	Tile();
	Tile(const std::string& name)// : Widget(name)
	{
		//board creation
	}

	~Tile();
	void Initialize();

	void SetTileType(TileDefinition* newTileDefintion){ m_tileDefinition = newTileDefintion;}
	Vector2 GetTileCoordinates(){ return m_tileCoords;}
	AABB2 GetBounds();

	void Render();

public:
	Vector2 m_tileCoords;
	TileDefinition* m_tileDefinition = nullptr;
	Tags m_tags;
	Rgba m_tint = Rgba::WHITE;
};