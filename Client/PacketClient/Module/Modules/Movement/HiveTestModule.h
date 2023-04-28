#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class HiveTestModule : public IModule {
private:
	float clipHeight = 2.f;
	bool clip = false;
	int counter69 = 0;
	int counter = 0;
	float fall = 1.f;
	float downf = 0.f;
	float falldist;
public:
	int HiveDelay = 3;
	float HiveVelocity = -0.05;

	// Hive
	bool velocityEffective = false;
	bool shouldFly = false;
	int hiveSpeedIndex = 0;
	int hiveVelocity = 0;
	int enabledTick = 0;
	int hiveTick = 0;
	int dmgTick = 0;
	float FallVelocity = 0;
	vec3_t initPos;



	virtual void onMove(C_MoveInputHandler* input);
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onSendPacket(C_Packet* packet);
	virtual void onDisable();
	virtual void onEnable();
	HiveTestModule();
};