#include "Game\Sprites\Sprite.hpp"
#include "Engine\Renderer\Renderer.hpp";
#include "Engine\Math\AABB2.hpp"

Sprite::Sprite(SpriteDefinition* definition)
{
	m_definition = definition;
}

IntVector2 Sprite::GetDimensions() const
{
	return IntVector2(m_definition->m_uvs.GetDimensions());
}

Texture* Sprite::GetSpriteTexture()
{
	Renderer* theRenderer = Renderer::GetInstance();

	return theRenderer->CreateOrGetTexture(m_definition->m_diffuseSource);
}

AABB2 Sprite::GetNormalizedUV()
{
	AABB2 normalizedUv = m_definition->m_uvs;

	if(m_definition->m_uvLayoutType == "pixel")
	{
		IntVector2 dimensions = GetSpriteTexture()->GetDimensions();

		float scaleX = 1.f / (float)dimensions.x;
		float scaleY = 1.f/ (float)dimensions.y;

		normalizedUv.mins.x *= scaleX;
		normalizedUv.maxs.x *= scaleX;
		normalizedUv.mins.y *= scaleY;
		normalizedUv.maxs.y *= scaleY;
	}	

	normalizedUv.mins.y = 1.f - normalizedUv.mins.y;
	normalizedUv.maxs.y = 1.f - normalizedUv.maxs.y;

	return normalizedUv;
}
