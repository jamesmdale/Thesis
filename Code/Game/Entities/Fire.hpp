#pragma once
#include "Engine\Math\IntVector2.hpp"
#include "Engine\Math\Vector2.hpp"

class Map;

class Fire
{
public:
	Fire(const IntVector2& coordinate, Map* map);
	~Fire();
	void Update(float deltaSeconds);
	inline bool IsDead(){ return m_health <= 0 ? true : false;}

public:
	int m_id = -1;
	int m_health = 100;
	IntVector2 m_coordinate;
	//IntVector2 m_accessCoordinate;
	Vector2 m_worldPosition;

	Map* m_mapReference = nullptr;

private:
	static int s_fireIdPosition;
};

