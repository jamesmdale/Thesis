#pragma once
#include "Engine\Math\Disc2.hpp"
#include "Engine\Time\Stopwatch.hpp"
#include "Engine\Math\Vector2.hpp"

class Bombardment
{
public:
	Bombardment(const Vector2& position);
	~Bombardment();

	void Update(float deltaSeconds);
	void Render();
	bool IsExplosionComplete();

public:
	Disc2 m_disc;
	Stopwatch* m_timer = nullptr;
};

