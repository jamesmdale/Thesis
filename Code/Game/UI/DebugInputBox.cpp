#include "Game\UI\DebugInputBox.hpp"
#include "Engine\Core\EngineCommon.hpp"
#include "Engine\Core\WindowsCommon.hpp"
#include "Engine\Window\Window.hpp"
#include "Engine\Core\Command.hpp"


static DebugInputBox* g_theDebugInputBox = nullptr;

//  =========================================================================================
bool DebugInputMessageHandler( unsigned int wmMessageCode, size_t wParam, size_t lParam ) 
{
	UNUSED(lParam);
	if(g_theDebugInputBox->IsOpen())
	{
		unsigned char asKey = (unsigned char) wParam;

		switch( wmMessageCode )
		{			
			// process keys that don't pass to wmchar
		case WM_CHAR:
		
			if(asKey == '\x1b') //escape
			{
				std::string input = g_theDebugInputBox->GetInput();
				if(input != "")
				{
					g_theDebugInputBox->ClearInput();
				}								
			}
			else if(asKey == '\r') //return
			{
				g_theDebugInputBox->ExecuteInput();
			}
			else if(asKey == '\b')
			{
				g_theDebugInputBox->RemoveLastCharacter();
			}
			//else if(asKey = )
			else
			{					
				if(g_theDebugInputBox->CheckIfValidInput((int)asKey))
				{
					g_theDebugInputBox->AppendCharacterToInput(asKey);
				}					
			}				
			return false;
			break;
		
		}
	}

	return true;	
}

//  =========================================================================================
DebugInputBox::DebugInputBox()
{
	SetAllowableCharacters();
	Window::GetInstance()->AddHandler( DebugInputMessageHandler ); 
}

//  =========================================================================================
DebugInputBox::~DebugInputBox()
{
	delete(g_theDebugInputBox);
	g_theDebugInputBox = nullptr;
}

//  =========================================================================================
DebugInputBox* DebugInputBox::CreateInstance()
{
	if (g_theDebugInputBox == nullptr)
	{
		g_theDebugInputBox = new DebugInputBox();
	}
	
	return g_theDebugInputBox;
}

//  =========================================================================================
DebugInputBox* DebugInputBox::GetInstance()
{
	return g_theDebugInputBox;
}

//  =========================================================================================
void DebugInputBox::SetAllowableCharacters()
{
	//add numerics
	for(int i = 48; i <= 57; i++)
		m_allowableCharacters.push_back(i);

	//add capital letters
	for(int i = 65; i <= 90; i++ )
		m_allowableCharacters.push_back(i);

	//add lower case letters
	for(int i = 97; i <= 122; i++ )
		m_allowableCharacters.push_back(i);

	//add all other accepted cases	
	m_allowableCharacters.push_back(32); //space
	m_allowableCharacters.push_back(34); //"
	m_allowableCharacters.push_back(40);//(
	m_allowableCharacters.push_back(41);//)
	m_allowableCharacters.push_back(44);//,
	m_allowableCharacters.push_back(46);//.
	m_allowableCharacters.push_back(58);//:
	m_allowableCharacters.push_back(61);//=
	m_allowableCharacters.push_back(95);//_
}

//  =========================================================================================
bool DebugInputBox::CheckIfValidInput(int asKey)
{
	for(int characterIndex = 0; characterIndex < (int)m_allowableCharacters.size(); characterIndex++) 
	{
		if(asKey == m_allowableCharacters[characterIndex])
		{
			return true;
		}
	}

	return false;
}

//  =========================================================================================
std::string DebugInputBox::GetInput()
{
	return m_currentInput;
}

//  =========================================================================================
void DebugInputBox::ExecuteInput()
{
	if(m_currentInput != "")
	{			
		bool isValidCommand = CommandRun(m_currentInput.c_str());
		ClearInput();
	}
}

//  =========================================================================================
bool DebugInputBox::IsOpen()
{
	return m_isOpen;
}

//  =========================================================================================
void DebugInputBox::Close()
{
	m_isOpen = false;
	ClearInput();
}

//  =========================================================================================
void DebugInputBox::Open()
{
	m_isOpen = true;
	ClearInput();
}

//  =========================================================================================
void DebugInputBox::ClearInput()
{
	m_currentInput = "";
}

//  =========================================================================================
void DebugInputBox::RemoveLastCharacter()
{
	if (m_currentInput.size() > 0)
	{
		m_currentInput.erase(m_currentInput.size() - 1);
	}

	m_currentInput.shrink_to_fit();
}

//  =========================================================================================
void DebugInputBox::AppendCharacterToInput(unsigned char asKey)
{
	if (m_currentInput.size() > MAX_INPUT_SIZE)
	{
		return;
	}

	m_currentInput.append(1, asKey);
	m_currentInput.shrink_to_fit();
}

