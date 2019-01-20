#pragma once
#include "Engine/Math/IntVector2.hpp"
#include <string>
#include <map>
#include "Engine\Core\Image.hpp"
#include "Game\Definitions\SpriteDefinitions\IsoSpriteDefinition.hpp"
#include "Game\Sprites\Sprite.hpp"
#include "Engine\Math\AABB2.hpp"

class IsoSprite
{
public:
	explicit IsoSprite(IsoSpriteDefinition* isoDefinition);
	IntVector2 IsoSprite::GetDimensionsForDirection(IntVector2 direction);
	IsoSpriteDefinition* GetIsoSpriteDefinition() const;
	AABB2 GetCurrentUVsByDirection(IntVector2 direction);
	Texture* GetCurrentTextureByDirection(IntVector2 direction);
	Sprite* GetCurrentSpriteByDirection(IntVector2 direction);
	IntVector2 GetCurrentSpriteScaleByDirection(IntVector2 direction);

private:
	IsoSpriteDefinition* m_definition;
	std::vector<Sprite*> m_sprites;
};

