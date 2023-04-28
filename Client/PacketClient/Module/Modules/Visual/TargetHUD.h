#pragma once
#include "../../../../Utils/TargetUtil.h"
#include "../../../../SDK/CAttribute.h"
#include "../../ModuleManager.h"
#include "../Module.h"

class TargetHUD : public IModule {
private:
	bool showItems = false;
	bool animation = true;
	int opacity = 100;
	
	float pct = 100.f;
	float displayhealth = 20.f;
	float displayhealth2 = 20.f;
	float size = 30.f;
	float animationsize = 1.f;
public:
	bool targetListEmpty = true;
	float targetLen = 0.f;
	float positionX = 10;
	float positionY = 40;
	int RiseQuality = 20;

	SettingEnum mode = this;

	virtual void onPostRender(C_MinecraftUIRenderContext* renderCtx);
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	TargetHUD();
};
