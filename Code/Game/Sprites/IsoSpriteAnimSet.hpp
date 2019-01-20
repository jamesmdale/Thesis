#pragma once
#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimSetDefinition.hpp"
#include <map>
#include "Engine\Renderer\Texture.hpp"
#include <string>
#include "Engine\Math\AABB2.hpp"
#include "Game\Sprites\IsoSpriteAnim.hpp"

class IsoSpriteAnimSet
{
public:
	IsoSpriteAnimSet( IsoSpriteAnimSetDefinition* animSetDef );
	~IsoSpriteAnimSet();

	void Update( float deltaSeconds );
	void StartAnim( const std::string& animNameSimplified );
	void IsoSpriteAnimSet::SetCurrentAnim( const std::string& animNameSimplified );
	const Texture& GetCurrentTexture(IntVector2 direction) const;
	AABB2 GetCurrentUVs(IntVector2 direction) const;
	IsoSpriteAnim* GetCurrentAnim() const {return m_currentAnim;};
	Sprite* GetCurrentSprite(IntVector2 direction);
	IsoSprite* GetCurrentIsoSprite();

protected:
	IsoSpriteAnimSetDefinition* m_animSetDef = nullptr;
	std::map< std::string, IsoSpriteAnim* > m_namedAnims;
	IsoSpriteAnim*	m_currentAnim = nullptr;
};
