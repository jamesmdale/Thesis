#pragma once
#include "Engine\Core\EngineCommon.hpp"
#include <vector>

class UtilityStorage
{
public:
	UtilityStorage(const float min, const float max, uint divisions);

	bool DoesValueExistForInput(const float input, float& outValue, int& outIndex);
	void StoreValueForInputAtIndex(const float calculatedValue, int index);
	void ResetData();

private:
	int CalculateIndexForInput(const float input);
	
private:
	float m_min = 0.f;
	float m_max = 0.f;
	uint m_divisions = 1;
	float m_incrementPerDivision = 0.f;
	std::vector<float> m_storage;
};