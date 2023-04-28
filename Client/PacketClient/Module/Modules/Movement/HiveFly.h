#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class HiveFly : public IModule {
private:
	vec3_t flypos;
	float clipHeight = 2.f;
	float speed = 1.f;
	bool clip = false;
	bool sprint = true;
	int counter69 = 0;
	int counter = 0;
	int ticks = 0;
	int delay = 0;
	int timer = 20;
	int canflytick = 5;
	float value = 0.f;
	float friction = 5.f;
public:

	SettingEnum mode = this;
	SettingEnum upmode = this;

	virtual void onMove(C_MoveInputHandler* input);
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onDisable();
	virtual void onEnable();
	HiveFly();
};