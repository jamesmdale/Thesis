#include "Game\EncounterState.hpp"
#include "Game\Map\Map.hpp"
#include "Engine\Time\Clock.hpp"



EncounterState::EncounterState()
{

}

EncounterState::~EncounterState()
{
	m_mapReference = nullptr;
}

void EncounterState::Update(float deltaSeconds)
{

}
