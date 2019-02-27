#include "Game\Entities\Fire.hpp"
#include "Engine\Renderer\Texture.hpp"
#include "Engine\Core\EngineCommon.hpp"


//  =========================================================================================
Fire::Fire(const int id, const IntVector2& coordinate)
{
	m_id = id;
	m_coordinate = coordinate;
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

