#pragma once
#include "../../../../Utils/TargetUtil.h"
#include "../../ModuleManager.h"
#include "../Module.h"

class Teams : public IModule {
private:
	bool alliedCheck = false;
	bool colorCheck = true;

public:
	bool SearchTeams = false;
	bool RemoveTeams = false;
	std::string name = "Teams";
	SettingEnum mode = this;
	Teams();

	bool isAlliedCheckEnabled() { return this->isEnabled() && alliedCheck; };
	bool isColorCheckEnabled() { return this->isEnabled() && colorCheck; };

	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onEnable();
};
