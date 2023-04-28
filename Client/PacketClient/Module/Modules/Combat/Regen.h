#pragma once

#include "../../ModuleManager.h"
#include "../Module.h"

class Regen : public IModule {
private:
    bool selectPickaxe();
    float animYaw = 0.f;
    vec3_ti blockPos;

    float range = 4.5f;
    int delay = 6;

    int tick = 0;
    int slot = 0;
public:
    bool hasDestroyed = false;
    bool destroy = false;
    int blocksBroke = 0;
    bool swing = false;
    bool healthvisual = true;
    bool bypassrot = false;
    bool firstbreak = false;
    bool continuemine = false;
    float animYawB;
    float animPitchB;
    bool smooth = false;
    bool rot = false;
    int rottime = 0;
    vec3_t OldBlockPos;
    int rsTick = 0;
    vec3_t tempblockPos;
    bool rotation = false;
    bool isregen = false;
    bool canattack = true;
    int enabledticks;
    int minedelay = 15;
    bool visual =false;
    bool bypass = true;
    bool breaknow = false;
    std::string name = "Regen";
    std::string name2 = "";
    SettingEnum mode = this;

    virtual void onPreRender(C_MinecraftUIRenderContext* renderCtx) override;
    virtual void onSendPacket(C_Packet* packet);
    virtual void onPlayerTick(C_Player* plr);
    virtual const char* getRawModuleName();
    virtual const char* getModuleName();
    virtual void onTick(C_GameMode* gm);
    virtual void onDisable();
    virtual void onEnable();
    Regen();
};
