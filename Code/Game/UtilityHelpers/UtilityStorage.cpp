#include "Game\UtilityHelpers\UtilityStorage.hpp"

//  =============================================================================
UtilityStorage::UtilityStorage(const float min, const float max, uint divisions)
{
	m_min = min;
	m_max = max;
	m_divisions = divisions;

	//resize and initialize all values in m_storage
	m_storage.resize((size_t)m_divisions);	
	for (int storageIndex = 0; storageIndex < (int)m_storage.size(); ++storageIndex)
	{
		m_storage[storageIndex] = FLT_MAX;
	}

	//calculate what the amount per division is
	m_incrementPerDivision = (m_max - m_min) / m_divisions;
}

//  =============================================================================
int UtilityStorage::CalculateIndexForInput(const float input)
{
	float mappedValue = RangeMapFloat(input, m_min, m_max, 0, m_divisions - 1);

	//same as a floor
	return (int)mappedValue;
}

//  =============================================================================
bool UtilityStorage::DoesValueExistForInput(const float input, float& outValue, int& outIndex)
{
	outIndex = CalculateIndexForInput(input);

	//if there is a value at this location, copy it as our output
	if (m_storage[outIndex] != FLT_MAX)
	{
		outValue = m_storage[outIndex];
		return true;
	}
	else
	{
		outValue = FLT_MAX;
		return false;
	}
}

//  =============================================================================
void UtilityStorage::StoreValueForInputAtIndex(const float calculatedValue, const int index)
{
	//if we don't already have a value at this location, store one
	if (m_storage[index] == FLT_MAX)
	{
		m_storage[index] = calculatedValue;
	}	
}

//  =============================================================================
void UtilityStorage::ResetData()
{
	m_storage.clear();
	
	//resize and initialize all values in m_storage
	m_storage.resize((size_t)m_divisions);	
	for (int storageIndex = 0; storageIndex < (int)m_storage.size(); ++storageIndex)
	{
		m_storage[storageIndex] = FLT_MAX;
	}
}
