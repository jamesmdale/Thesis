#pragma once
#include "Engine\Core\Command.hpp"
#include "Game\GameCommon.hpp"
#include "Engine\Core\EngineCommon.hpp"

class TheApp
{
public:
	TheApp();
	~TheApp();
	void Update();
	void PreRender();
	void Render();
	void PostRender();
	void Initialize();
	void RunFrame();
	float UpdateInput(float deltaSeconds);

	//resoureces
	void InitializeGameResources();
};

void Quit(Command &cmd);
void WriteTestCSV();

extern TheApp* g_theApp;

