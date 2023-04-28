#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class ClickTP : public IModule {
private:
	bool hasClicked = false;
	int timeWaited = 1;
	bool hand = true;
	bool servermode = false;
	vec3_t position;

public:
	virtual void onPostRender(C_MinecraftUIRenderContext* renderCtx) override;
	virtual void onTick(C_GameMode* gm) override;
	virtual const char* getModuleName() override;
	ClickTP();
};