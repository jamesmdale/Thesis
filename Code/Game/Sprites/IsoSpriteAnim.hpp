#include "Game\Definitions\SpriteDefinitions\IsoSpriteAnimDefinition.hpp"
#include "Engine\Renderer\Texture.hpp"
#include "Engine\Math\AABB2.hpp"
#include "Game\Sprites\IsoSprite.hpp"

class IsoSpriteAnim
{
public:
	IsoSpriteAnim( IsoSpriteAnimDefinition* animDef, float totalAnimationTime);
	~IsoSpriteAnim();

	void Update( float deltaSeconds );
	void PlayFromStart();
	bool IsFinished() const { return m_isFinished; }
	float GetElapsedSeconds() const { return m_elapsedSeconds; }
	float GetFractionElapsed() const; // Hint: Asks its SpriteAnimDefinition for total duration
	float GetSecondsRemaining() const;
	float GetFractionRemaining() const;
	float GetFractionElapsedForTime(float time) const;
	void SetSecondsElapsed( float secondsElapsed );
	void SetFractionElapsed( float fractionElapsed );
	void SetTotalTime(float totalTimeToPlay);

	int GetIsoSpriteIndexForTime(float time);

	int GetCurrentIsoSpriteIndex();
	std::string GetName() const;

	//TODO: Might be some useful functions later
	//void Resume()
	//void Pause()
	//	const Texture* GetTexture(IntVector2 direction) const;

protected:
	IsoSpriteAnimDefinition* m_animDef = nullptr;
	int m_currentIsoSpriteIndex;
	bool m_isPlaying = true;
	bool m_isFinished = false;
	float m_elapsedSeconds = 0.f;
	float m_totalAnimationTime = 0.f;

public:
	std::vector<IsoSprite*> m_isoSprites;
};
