#pragma once
#include "../../ModuleManager.h"
#include "../../Utils/DrawUtils.h"
#include "../Module.h"

class AntiVoid : public IModule {
private:
	std::vector<vec3_t> below;
	bool foundVoid = false;
	bool voidCheck = true;
	bool tponce = false;
	int distance = 5;
	vec3_t savedPos;
	int tick = 0;
	bool tped = false;
	int stoptime = 0;
	bool scaffff = true;

public:
	bool blink = false;
	bool lockis = false;

	C_MoveInputHandler* inputHandler = nullptr;
	SettingEnum mode = this;

	virtual void onMove(C_MoveInputHandler* input);
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onDisable();
	virtual void onEnable();
	AntiVoid();
};
