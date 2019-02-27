#pragma once
#include "Engine\Math\IntVector2.hpp"

class Fire
{
public:
	Fire(const int id, const IntVector2& coordinate);
	~Fire();
	void Update(float deltaSeconds);
	inline bool IsDead(){ return m_health <= 0 ? true : false;}

public:
	int m_id = -1;
	int m_health = 100;
	IntVector2 m_coordinate;
};

