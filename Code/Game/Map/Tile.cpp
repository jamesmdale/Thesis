#include "Game\Map\Tile.hpp"
#include "Game\GameCommon.hpp"
#include "Engine\Renderer\Renderer.hpp"
#include "Engine\Renderer\Renderable2D.hpp"
#include "Engine\Renderer\MeshBuilder.hpp"
#include "Engine\Window\Window.hpp"
#include "Game\GameCommon.hpp"
#include "Engine\Core\StringUtils.hpp"

//  =========================================================================================
Tile::Tile()
{
}

//  =========================================================================================
Tile::~Tile()
{
	m_tileDefinition = nullptr;
}

//  =========================================================================================
void Tile::Initialize()
{

}

//  =========================================================================================
AABB2 Tile::GetBounds()
{
	Vector2 minCoords = GetWorldCoordinates();
	return AABB2(minCoords, minCoords + Vector2::ONE);	
}

//  =========================================================================================
void Tile::Render()
{
	Renderer* theRenderer = Renderer::GetInstance();

	theRenderer->DrawTexturedAABB(GetBounds(), *theRenderer->CreateOrGetTexture("Data/Images/Terrain_8x8.png"), m_tileDefinition->m_baseSpriteUVCoords.mins, m_tileDefinition->m_baseSpriteUVCoords.maxs, m_tint);
	
	theRenderer->SetTexture(*theRenderer->CreateOrGetTexture("default"));

	theRenderer = nullptr;
}
