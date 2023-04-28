#include "Notifications.h"
#include "../pch.h"

Notifications::Notifications() : IModule(0, Category::VISUAL, "Displays notifications") {
	registerEnumSetting("Theme", &mode, 0);
	mode.addEntry("Packet", 0);
	mode.addEntry("Tenacity", 1);
	registerBoolSetting("ShowToggle", &showToggle, showToggle);
	registerBoolSetting("Color", &color, color);
	registerIntSetting("Opacity", &opacity, opacity, 0, 255);
	shouldHide = true;
}

const char* Notifications::getModuleName() {
	return ("Notifications");
}
