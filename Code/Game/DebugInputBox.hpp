#pragma once
#include <string.h>
#include <vector>

constexpr int MAX_INPUT_SIZE = 50;

class DebugInputBox
{
public:
	DebugInputBox();
	~DebugInputBox();

	static DebugInputBox* CreateInstance();
	static DebugInputBox* GetInstance();

	void SetAllowableCharacters();
	bool CheckIfValidInput(int asKey);

	bool IsOpen();
	void Close();
	void Open();

	std::string GetInput();
	void ExecuteInput();
	void ClearInput();
	void RemoveLastCharacter();
	void AppendCharacterToInput(unsigned char asKey);

public:
	std::vector<int> m_allowableCharacters;
	
private:
	std::string m_currentInput;
	bool m_isOpen = false;
};

bool DebugInputMessageHandler(unsigned int wmMessageCode, size_t wParam, size_t lParam);
