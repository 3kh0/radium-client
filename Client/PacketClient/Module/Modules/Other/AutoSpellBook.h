#pragma once
#include "../../ModuleManager.h"
#include "../Module.h"

class AutoSpellBook : public IModule {
private:
    //float animYaw = 0.f;
public:
    std::string name = "AutoSpellBook";
    //SettingEnum mode = this;

    // Inherited via IModule
    //virtual void onMove(C_MoveInputHandler* input) override;
    //virtual void onSendPacket(C_Packet* packet) override;
    virtual void onPlayerTick(C_Player* plr) override;
    //virtual const char* getRawModuleName() override;
    virtual const char* getModuleName() override;
    virtual void onTick(C_GameMode* gm) override;
    //virtual void onEnable() override;
    AutoSpellBook();
};