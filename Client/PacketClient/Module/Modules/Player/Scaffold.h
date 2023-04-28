#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class Scaffold : public IModule {
private:
	int offGroundTicks = 0;
	bool uwu = false;
	bool awa = false;

	int intPosY = 0;
	bool placeMoreUnder = false;
	bool shouldSwing = false;
	bool towerOnlyNoMove = false;
	bool foundBlock = false;
	bool blockCount = true;
	bool showExposed = true;
	bool movedx = false;
	bool movedz = false;
	bool speedwasenabled = false;
	vec3_t oldpos;
	int expR = 255;
	int expG = 50;
	int expB = 0;
	int delay = 0;
	int Odelay = 0;
	float expT = 0.4f;
	bool inclSelf = true;
	float animPitch = 0.f;
	float animBack = 0.f;
	float animFlareon = 0.f;
	float animFlareonPitch = 0.f;
	float animYaw = 0.f;
	int towerTimer = 20;
	int towerTick = 0;
	float towerMultiply = 1.2f;
	float random = 1.5f;
	bool sprint = true;
	bool zipline = false;
	bool preventkicks = false;
	bool firstlock = false;
	int yLock = -1;
	int timer = 20;
	int range = 4;
	int slot = 0;
	float telly = 0.1;
	int fakespoofticks = 0;
	int rundown;
	vec3_t blockBelowY;

	bool sneaking = false;
	bool jumping = false;
	bool oneDot12 = true;
	bool digbypass = false;

	bool findBlocks(C_ItemStack* itemStack);
	bool isUsefulBlock(C_ItemStack* itemStack);
	bool buildBlock(vec3_t blockBelow);
	bool predictBlock(vec3_t blockBelow);
	bool isBlockReplacable(vec3_t blockPos);
	bool selectBlock();

	// Positions
	vec3_t calcPos = vec3_t{ 0, 0, 0 };
	vec3_t initialPos;
	vec3_t blockPos;
	vec3_t backPos;
	vec3_t flareonpos;
	vec3_t flareonpos2;

public:
	float currExtend = 0.f;
	bool swing = true;
	float Range = 4;
	float scY = 20.f;
	float scX = 10.f;
	int extend = 1;
	int minetime = 15;
	int groundtime = 0;
	int groundtime2 = 0;
	int tellytick = 0;
	int tellydalay = 5;
	int rotspeed = 10;
	bool telly2 = false;
	bool lockY = false;
	bool canspoof = false;
	bool downmode = false;

	std::vector<vec3_t> placed;
	std::vector<vec3_ti> highlightCity;
	std::vector<vec3_t> exposee;
	std::string name = ("Scaffold");
	SettingEnum rotations = this;
	SettingEnum tower = this;
	SettingEnum placemode = this;
	SettingEnum holdType = this;
	SettingEnum extendType = this;
	bool isBlockAGoodCity(vec3_ti* blk, vec3_ti* personPos);

	virtual void onPreRender(C_MinecraftUIRenderContext* renderCtx) override;
	virtual void onMove(C_MoveInputHandler* input) override;
	virtual void onSendPacket(C_Packet* packet) override;
	virtual void onPlayerTick(C_Player* plr) override;
	virtual const char* getRawModuleName() override;
	virtual const char* getModuleName() override;
	virtual void onTick(C_GameMode* gm) override;
	//virtual void onWorldTick(C_GameMode* gm) override;
	virtual void onDisable() override;
	virtual void onEnable() override;
	Scaffold();
};
