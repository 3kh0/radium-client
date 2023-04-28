#pragma once
#include "../../ModuleManager.h"
#include "../../Utils/DrawUtils.h"
#include "../Module.h"

class Interface : public IModule {
public:
	std::string clientName = "Radium";
	std::string releaseStr = "Release";
	std::string versionStr;

	std::string& getClientName() { return clientName; };
public:
	// Settings
	bool keystrokes = false;
	float saturation = 1.f;
	bool armorHUD = false;
	bool release = true;
	bool tabGUI = false;
	int opacity = 150;
	int coloropacity = 255;
	bool info = true;
	bool ping = true;
	bool fps = true;
	bool speed = true;
	bool position = true;
	int spacing = 70;

	float scale = 1.f;

	// Positions
	vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
	float releaseX = 2.5;
	float releaseY = 2.5;
	float armorX = 2.5;
	float armorY = 60;
	float fpsX = 2.5;
	float fpsY = windowSize.y - 40;
	float pingX = 2.5;
	float pingY = windowSize.y - 50;
	float RegenX = windowSize.x / 2;
	float RegenY = windowSize.y - 60;
	float bpsX = 2.5f;
	float bpsY = windowSize.y - 30;
	float posX = 2.5f;
	float posY = windowSize.y - 20;

	// Colors
	SettingEnum color = this;
	SettingEnum Fonts = this;
	float r = 255.f, g = 255.f, b = 255.f;
	float r2 = 32.f, g2 = 32.f, b2 = 32.f;

	virtual void onPostRender(C_MinecraftUIRenderContext* renderCtx);
	virtual void onPreRender(C_MinecraftUIRenderContext* renderCtx);
	virtual const char* getModuleName();
	virtual void onTick(C_GameMode* gm);
	virtual void onDisable();
	Interface();
};
