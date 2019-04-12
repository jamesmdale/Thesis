#include "Game\Entities\Bombardment.hpp"
#include "Game\Game.hpp"
#include "Game\GameCommon.hpp"
#include "Engine\Profiler\Profiler.hpp"
#include "Engine\Renderer\Renderer.hpp"


//  =========================================================================================
Bombardment::Bombardment(const Vector2& position)
{
	m_disc.center = position;
	m_disc.radius = 0.f;

	m_timer = new Stopwatch(GetGameClock());
	m_timer->SetTimer(g_bombardmentExplosionTime);
}

//  =========================================================================================
Bombardment::~Bombardment()
{
	delete(m_timer);
	m_timer = nullptr;
}

//  =========================================================================================
void Bombardment::Update(float deltaSeconds)
{
	//PROFILER_PUSH();

	float percentComplete = m_timer->GetNormalizedElapsedTimeInSeconds();

	m_disc.radius = g_bombardmentExplosionSize * percentComplete;
}

//  =========================================================================================
void Bombardment::Render()
{
	//PROFILER_PUSH();

	Renderer* theRenderer = Renderer::GetInstance();

	theRenderer->SetShader(theRenderer->CreateOrGetShader("agents"));
	theRenderer->DrawTexturedAABB(AABB2(m_disc.center, m_disc.radius, m_disc.radius), *theRenderer->CreateOrGetTexture("Data/Images/AirStrike.png"), Vector2::ZERO, Vector2::ONE, Rgba(1.f, 1.f, 1.f, .5f));
	theRenderer->SetShader(theRenderer->m_defaultShader);
}

//  =========================================================================================
bool Bombardment::IsExplosionComplete()
{
	return m_timer->HasElapsed();
}
