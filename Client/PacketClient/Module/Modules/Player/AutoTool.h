#pragma once

#include "../../ModuleManager.h"
#include "../Module.h"

class AutoTool : public IModule {
public:
	std::string name = "AutoTool";
	bool hasClicked = false;
	int prevslot = 0;
	bool weapon = false;
	bool tools = true;
	// Inherited via IModule
	virtual const char* getModuleName() override;
	virtual void onTick(C_GameMode* gm) override;
	virtual void onEnable() override;
	virtual void onAttack(C_Entity* attackedEnt) override;
	AutoTool();
};
