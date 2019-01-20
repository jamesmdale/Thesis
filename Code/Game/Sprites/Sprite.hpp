#pragma once
#include "Engine\Math\IntVector2.hpp"
#include <string>
#include <map>
#include "Engine\Core\Image.hpp"
#include "Game\Definitions\SpriteDefinitions\SpriteDefinition.hpp"
#include "Engine\Math\Vector2.hpp"
#include "Engine\Renderer\Texture.hpp"

class Sprite
{
public:
	explicit Sprite(SpriteDefinition* definition);
	//Sprite(const std::string& diffusePath, const int& pixelsPerUnit, const AABB2& uv, const Vector2& pivot);
	IntVector2 GetDimensions() const;
	Texture* GetSpriteTexture();
	AABB2 GetNormalizedUV();

public:
	SpriteDefinition* m_definition;
};


