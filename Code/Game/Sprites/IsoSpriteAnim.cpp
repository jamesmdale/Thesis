#include "Game\Sprites\IsoSpriteAnim.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Profiler\Profiler.hpp"

//  =============================================================================
IsoSpriteAnim::IsoSpriteAnim( IsoSpriteAnimDefinition* animDef, float totalAnimationTime)
{
	m_animDef = animDef;
	m_elapsedSeconds = 0.0f;
	m_isPlaying = true;
	m_isFinished = false;
	m_totalAnimationTime = animDef->GetDuration();

	for(int isoSpriteIndex = 0; isoSpriteIndex < animDef->m_frameStructs.size(); isoSpriteIndex++)
	{
		IsoSpriteDefinition* isoSriteDefinition = GetIsoSpriteDefinitionById(m_animDef->m_frameStructs[isoSpriteIndex].m_frameSource);

		if(isoSriteDefinition != nullptr)
		{
			TODO("Could manage this like a texutre and do a CreateOrGetIsoSprite/CreateOrGetSprite so that we aren't creating identical copies of the same sprites across animations");
			m_isoSprites.push_back(new IsoSprite(isoSriteDefinition));
		}
	}
}

//  =============================================================================
void IsoSpriteAnim::Update( float deltaSeconds )
{
	//PROFILER_PUSH();

	m_elapsedSeconds += deltaSeconds;
	float fractionelapsed = GetFractionElapsed();
	if(fractionelapsed >= 1.f)
	{
		if(m_animDef->m_doesLoop == true)
		{
			m_elapsedSeconds = 0.0f;
			PlayFromStart();
		}
		else
		{
			m_isFinished = true;
			m_isPlaying = false;		
		}				
	}
	else
	{
		m_currentIsoSpriteIndex = GetCurrentIsoSpriteIndex();
	}
}

//  =============================================================================
int IsoSpriteAnim::GetCurrentIsoSpriteIndex()	
{
	return GetIsoSpriteIndexForTime(m_elapsedSeconds);
}

//  =============================================================================
void IsoSpriteAnim::PlayFromStart()
{
	m_elapsedSeconds = 0.0f;
	m_isFinished = false;
	m_isPlaying = true;	
}

//  =============================================================================
float IsoSpriteAnim::GetSecondsRemaining() const
{
	return m_totalAnimationTime - m_elapsedSeconds;
}

//  =============================================================================
float IsoSpriteAnim::GetFractionElapsed() const
{
	return m_elapsedSeconds/m_animDef->GetDuration();
}

//  =============================================================================
float IsoSpriteAnim::GetFractionRemaining() const
{
	return 1.0f - (m_elapsedSeconds/m_totalAnimationTime);
}

//  =============================================================================
float IsoSpriteAnim::GetFractionElapsedForTime(float time) const
{
	if(m_totalAnimationTime == 0)
	{
		return 0.f;
	}

	return time/m_totalAnimationTime;
}

//  =============================================================================
void IsoSpriteAnim::SetSecondsElapsed( float secondsElapsed )   
{
	m_elapsedSeconds = secondsElapsed;
}

//  =============================================================================
void IsoSpriteAnim::SetFractionElapsed( float fractionElapsed )
{
	m_elapsedSeconds = fractionElapsed * m_totalAnimationTime;
}

//  =============================================================================
std::string IsoSpriteAnim::GetName() const
{
	return m_animDef->m_id;
}

//  =============================================================================
int IsoSpriteAnim::GetIsoSpriteIndexForTime(float time)
{
	//float percentageElapsed = GetFractionElapsedForTime(time);
	float currentSum = 0.f;

	for(int isoSpriteIndex = 0; isoSpriteIndex < (int)m_animDef->m_frameStructs.size(); isoSpriteIndex++)
	{
		currentSum += m_animDef->m_frameStructs[isoSpriteIndex].m_animationTimePercentage;

		if(m_elapsedSeconds <= currentSum)
		{
			return isoSpriteIndex;
		}
	}

	return (int)m_animDef->m_frameStructs.size() - 1;
}

//  =============================================================================
void IsoSpriteAnim::SetTotalTime(float totalTimeToPlay)
{
	m_totalAnimationTime = totalTimeToPlay;
}