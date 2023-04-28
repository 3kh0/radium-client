#include "AutoSave.h"

using namespace std;
AutoSave::AutoSave() : IModule(0, Category::OTHER, "Automatically save config") {
    registerEnumSetting("When", &when, 0);
    when.addEntry("CloseClickGUI", 0);
}

const char* AutoSave::getModuleName() {
    return "AutoSave";
}

void AutoSave::onEnable() {
}

void AutoSave::onTick(C_GameMode* gm) {
}