#include "Game\Sprites\IsoSprite.hpp"
#include "Engine\Renderer\Renderer.hpp";
#include "Engine\Math\AABB2.hpp"

IsoSprite::IsoSprite(IsoSpriteDefinition* definition)
{
	m_definition = definition;
	
	for(int structIndex = 0; structIndex < (int)m_definition->m_facingStructs.size(); structIndex++)
	{
		SpriteDefinition* spriteDefinition = GetSpriteDefinitionById(m_definition->m_facingStructs[structIndex].m_src);

		if(spriteDefinition != nullptr)
		{
			m_sprites.push_back(new Sprite(spriteDefinition));
		}
	}
}

IntVector2 IsoSprite::GetDimensionsForDirection(IntVector2 direction)
{
	std::string source = m_definition->GetSourceByDirection(direction);
	
	for(int spriteIndex = 0; spriteIndex < (int)m_sprites.size(); spriteIndex++)
	{
		SpriteDefinition* spriteDefinition = m_sprites[spriteIndex]->m_definition;
		if(source == spriteDefinition->m_id)
		{
			spriteDefinition = nullptr;

			return m_sprites[spriteIndex]->GetDimensions();
		}
		spriteDefinition = nullptr;
	}

	return IntVector2(0,0);
}

IsoSpriteDefinition* IsoSprite::GetIsoSpriteDefinition() const
{
	return m_definition;
}

AABB2 IsoSprite::GetCurrentUVsByDirection(IntVector2 direction)
{
	std::string source = m_definition->GetSourceByDirection(direction);

	for(int spriteIndex = 0; spriteIndex < (int)m_sprites.size(); spriteIndex++)
	{
		SpriteDefinition* spriteDefinition = m_sprites[spriteIndex]->m_definition;
		if(source == spriteDefinition->m_id)
		{
			spriteDefinition = nullptr;

			return m_sprites[spriteIndex]->m_definition->m_uvs;
		}
		spriteDefinition = nullptr;
	}

	return AABB2();
}

Texture* IsoSprite::GetCurrentTextureByDirection(IntVector2 direction)
{
	std::string source = m_definition->GetSourceByDirection(direction);

	for(int spriteIndex = 0; spriteIndex < (int)m_sprites.size(); spriteIndex++)
	{
		SpriteDefinition* spriteDefinition = m_sprites[spriteIndex]->m_definition;
		if(source == spriteDefinition->m_id)
		{
			spriteDefinition = nullptr;

			return m_sprites[spriteIndex]->GetSpriteTexture();
			 
		}
		spriteDefinition = nullptr;
	}

	return nullptr;
}

Sprite* IsoSprite::GetCurrentSpriteByDirection(IntVector2 direction)
{
	std::string source = m_definition->GetSourceByDirection(direction);

	for(int spriteIndex = 0; spriteIndex < (int)m_sprites.size(); spriteIndex++)
	{
		SpriteDefinition* spriteDefinition = m_sprites[spriteIndex]->m_definition;
		if(source == spriteDefinition->m_id)
		{
			spriteDefinition = nullptr;

			return m_sprites[spriteIndex];

		}
		spriteDefinition = nullptr;
	}
}

IntVector2 IsoSprite::GetCurrentSpriteScaleByDirection(IntVector2 direction)
{
	return m_definition->GetScaleByDirection(direction);
}