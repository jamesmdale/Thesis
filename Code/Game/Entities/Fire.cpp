#include "Game\Entities\Fire.hpp"
#include "Engine\Renderer\Texture.hpp"
#include "Engine\Core\EngineCommon.hpp"

int Fire::s_fireIdPosition = 0;

//  =========================================================================================
Fire::Fire(const int id, const IntVector2& coordinate)
{
	m_id = s_fireIdPosition;
	m_coordinate = coordinate;
	m_worldPosition = Vector2(coordinate) + Vector2(0.5f, 0.5f);

	s_fireIdPosition++;
}

//  =========================================================================================
Fire::~Fire()
{

}

//  =========================================================================================
void Fire::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

