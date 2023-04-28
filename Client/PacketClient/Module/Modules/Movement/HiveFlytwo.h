#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class HiveFlytwo : public IModule {
private:
	bool dashed = false;
	bool aurais = false;
	bool disableris = false;
	bool speedis = false;
	bool lock = false;
	bool cliplimit = false;
	float speed = 0.5;
	float height = 0.0;
	float clipUp = 1.0;
	float clipvalue = 0.0;
	int timer = 20;
	int dashTime = 0;
	int cliptimes = 4;
	float upsped = 2;
	vec3_t savePos;
	int nowtimes;
	bool aids = false;
	float upheight;
	float dspeed = 0.f;
	float duration = 1.f;
	std::string name = "HiveFly2";
	SettingEnum mode = this;
public:
	virtual void onMove(C_MoveInputHandler* input);
	virtual const char* getRawModuleName();
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onDisable();
	virtual void onEnable();
	HiveFlytwo();
};
