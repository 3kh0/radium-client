#pragma once
#include "../../../../Utils/HMath.h"
#include "../../ModuleManager.h"
#include "../Module.h"

class Bhop : public IModule {
private:
	int offGroundTicks = 0;
	int groundTicks = 0;
public:
	float effectiveSpeed2 = 0.5f;
	float duration2 = 1.f;
	int timer2 = 20;
	int ticks2 = 0;
	bool horizontal = false;
	bool fastfall = false;

	// Hive
	float speedFriction2 = 0.65f;
	float maxSpeed2 = 0.98883f;
	float maxSpeed3 = 0.88991f;
	bool preventKick2 = false;

	std::string name = "Bhop";
	SettingEnum mode = this;

	inline std::vector<PlayerAuthInputPacket*>* getPlayerAuthInputPacketHolder() { return &PlayerAuthInputPacketHolder; };
	inline std::vector<C_MovePlayerPacket*>* getMovePlayerPacketHolder() { return &MovePlayerPacketHolder; };
	std::vector<PlayerAuthInputPacket*> PlayerAuthInputPacketHolder;
	std::vector<C_MovePlayerPacket*> MovePlayerPacketHolder;
	virtual void onMove(C_MoveInputHandler* input);
	virtual void onSendPacket(C_Packet* packet);
	virtual const char* getRawModuleName();
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onDisable();
	virtual void onEnable();
	Bhop();
};