#include "Game\Sprites\IsoSpriteAnimSet.hpp"
#include "Engine\Core\StringUtils.hpp"
#include "Engine\Profiler\Profiler.hpp"

//  =========================================================================================
IsoSpriteAnimSet::IsoSpriteAnimSet( IsoSpriteAnimSetDefinition* animSetDef )
{
	m_animSetDef = animSetDef;

	std::map<std::string, IsoSpriteAnimDefinition*>::iterator defIterator = IsoSpriteAnimDefinition::s_isoSpriteAnimDefinitions.begin();

	// Iterate over the map using Iterator till end.
	while (defIterator != IsoSpriteAnimDefinition::s_isoSpriteAnimDefinitions.end())
	{	
		IsoSpriteAnim* newAnim = new IsoSpriteAnim(defIterator->second, 1.0f);
		m_namedAnims[defIterator->first] = newAnim;
		defIterator++;
	}

	SetCurrentAnim("idle"); //idle is our default.  If we don't find and idle, it doesn't set one.
}

//  =========================================================================================
void IsoSpriteAnimSet::Update( float deltaSeconds )
{
	PROFILER_PUSH();

	//TODO: add logic determining which anim to run from here.  This is the animation related pointer held in the entity itself.
	m_currentAnim->Update(deltaSeconds);

	if(m_currentAnim->IsFinished())
	{
		SetCurrentAnim("idle");
	}
}

//  =========================================================================================
void IsoSpriteAnimSet::StartAnim(const std::string& animNameSimplified )
{
	SetCurrentAnim(animNameSimplified);
	m_currentAnim->PlayFromStart();
}

//  =========================================================================================
void IsoSpriteAnimSet::SetCurrentAnim( const std::string& animNameSimplified )
{
	std::string animName = m_animSetDef->GetAnimationIdBySimplifiedName(ToLowerAsNew(animNameSimplified));

	auto animIndex = m_namedAnims.find(animName);
	if(animIndex != m_namedAnims.end())
	{
		if (m_currentAnim != m_namedAnims.at(animName))
		{
			m_currentAnim = m_namedAnims.at(animName);
			m_currentAnim->PlayFromStart();
		}			
	}
	else
	{
		m_currentAnim = m_namedAnims.at("idle");
		m_currentAnim->PlayFromStart();
	}
}

//  =========================================================================================
const Texture& IsoSpriteAnimSet::GetCurrentTexture(IntVector2 direction) const
{
	int index = m_currentAnim->GetIsoSpriteIndexForTime(m_currentAnim->GetElapsedSeconds());
	IsoSprite* isoSprite = m_currentAnim->m_isoSprites[index];

	return *isoSprite->GetCurrentTextureByDirection(direction);
}

//  =========================================================================================
AABB2 IsoSpriteAnimSet::GetCurrentUVs(IntVector2 direction) const
{
	int index = m_currentAnim->GetIsoSpriteIndexForTime(m_currentAnim->GetElapsedSeconds());
	IsoSprite* isoSprite = m_currentAnim->m_isoSprites[index];

	AABB2 uvs = isoSprite->GetCurrentUVsByDirection(direction);

	return uvs;
}

//  =========================================================================================
Sprite* IsoSpriteAnimSet::GetCurrentSprite(IntVector2 direction)
{
	IsoSpriteAnim* anim = GetCurrentAnim();
	int currentIsoSpriteIndex = anim->GetCurrentIsoSpriteIndex();

	Sprite* sprite = anim->m_isoSprites[currentIsoSpriteIndex]->GetCurrentSpriteByDirection(direction);

	return sprite;
}

//  =========================================================================================
IsoSprite* IsoSpriteAnimSet::GetCurrentIsoSprite()
{
	IsoSpriteAnim* anim = GetCurrentAnim();
	int currentIsoSpriteIndex = anim->GetCurrentIsoSpriteIndex();
	return anim->m_isoSprites[currentIsoSpriteIndex];	
}