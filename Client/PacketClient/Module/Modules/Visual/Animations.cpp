#include "Animations.h"
#include "../pch.h"
#include "../../../../Memory/Hooks.h"

void* SmoothSwing = (void*)FindSignature("0F 84 ? ? ? ? 48 8B 56 ? 48 85 D2 74 ? 48 8B 02");
void* TapAddress = (void*)FindSignature("F3 0F 51 F0 0F 28 C8");

using namespace std;
Animations::Animations() : IModule(0, Category::VISUAL, "Changes the swing/hitting animation") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("None", 0);
	mode.addEntry("1.7", 1);
	mode.addEntry("Slide", 2);
	mode.addEntry("Old", 3);
	mode.addEntry("Spin", 4);
	mode.addEntry("1.7+", 5);
	mode.addEntry("Exhi", 6);
	//registerBoolSetting("SmoothSwing", &smoothSwing, smoothSwing);
	registerEnumSetting("Swing", &type, 0);
	type.addEntry("None", 0);
	type.addEntry("Flux", 1);
	type.addEntry("Tap", 2);
	type.addEntry("Exhi", 3);
	//registerBoolSetting("TapSwing", &tapswing, tapswing);
	registerBoolSetting("NoSwing", &noswing, noswing);
	registerBoolSetting("NoSwingV2", &slowSwing, slowSwing);
	registerBoolSetting("Swing", &swing, swing);
	registerBoolSetting("Reset", &reset, reset);
	registerFloatSetting("X", &xPos, xPos, -3.f, 3.f);
	registerFloatSetting("Y", &yPos, yPos, -3.f, 3.f);
	registerFloatSetting("Z", &zPos, zPos, -3.f, 3.f);
}

const char* Animations::getRawModuleName() {
	return "Animations";
}

const char* Animations::getModuleName() {
	if (mode.getSelectedValue() == 0) name = string("Animations");
	if (mode.getSelectedValue() == 1) name = string("Animations ") + string(GRAY) + string("1.7");
	if (mode.getSelectedValue() == 2) name = string("Animations ") + string(GRAY) + string("Slide");
	if (mode.getSelectedValue() == 3) name = string("Animations ") + string(GRAY) + string("Old");
	if (mode.getSelectedValue() == 4) name = string("Animations ") + string(GRAY) + string("Spin");
	if (mode.getSelectedValue() == 5) name = string("Animations ") + string(GRAY) + string("1.7+");
	if (mode.getSelectedValue() == 6) name = string("Animations ") + string(GRAY) + string("Exhi");
	return name.c_str();
}

void Animations::onEnable() {
}

void Animations::onTick(C_GameMode* gm) {
	auto killaura = moduleMgr->getModule<Killaura>();
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	requireswing = false;

	auto slot = player->getSupplies()->inventory->getItemStack(player->getSupplies()->selectedHotbarSlot);
	shouldBlock = slot != nullptr && slot->item != nullptr && slot->getItem()->isWeapon() && g_Data.isRightClickDown() || slot != nullptr && slot->item != nullptr && slot->getItem()->isWeapon() && !killaura->targetListEmpty && killaura->isEnabled();
	isAttacking = g_Data.isLeftClickDown() || (!killaura->targetListEmpty && killaura->isEnabled());

	if (type.getSelectedValue() == 3) {
		Utils::nopBytes((unsigned char*)SmoothSwing, 6);
		Utils::nopBytes((BYTE*)TapAddress, 4);
	}
	else {
		if (type.getSelectedValue() != 1) Utils::patchBytes((unsigned char*)((uintptr_t)SmoothSwing), (unsigned char*)"\x0F\x84\x95\x02\x00\x00", 6);
		if(type.getSelectedValue() != 2) Utils::patchBytes((BYTE*)((uintptr_t)TapAddress), (BYTE*)"\xF3\x0F\x51\xF0", 4);
	}

	if (type.getSelectedValue() == 1) Utils::nopBytes((unsigned char*)SmoothSwing, 6);
	else if(type.getSelectedValue() != 3) Utils::patchBytes((unsigned char*)((uintptr_t)SmoothSwing), (unsigned char*)"\x0F\x84\x95\x02\x00\x00", 6);

	if(type.getSelectedValue() == 2) Utils::nopBytes((BYTE*)TapAddress, 4);
	else if(type.getSelectedValue() != 3)
	{
		Utils::patchBytes((BYTE*)((uintptr_t)TapAddress), (BYTE*)"\xF3\x0F\x51\xF0", 4);
	}

	if (shouldBlock) {
		//if (mode.getSelectedValue() == 1) Utils::nopBytes((unsigned char*)SmoothSwing, 6);
		if (!swing) *player->getSwingState() = 0.f;
	}

	if (slowSwing) *player->getSwingState() = 0.f;

	if (reset) { 
		xScale = 1.f; yScale = 1.f; zScale = 1.f; 
		xPos = 0.f; yPos = 0.f; zPos = 0.f; 
		reset = false;
	}
}

float swingSpeed = 0.f;
void Animations::onPlayerTick(C_Player* plr) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (swingSpeed >= 2005) swingSpeed = 340;
	else swingSpeed += 1.f;
	if (swingSpeed <= 339) swingSpeed = 340;

	if (g_Data.canUseMoveKeys() && !moduleMgr->getModule<ClickGUIMod>()->isEnabled()/* && g_Data.isLeftClickDown()*/) {
		requireswing = true;
	}

	if (requireswing) {
		if (slowSwing) {
			if (swingticks > 2005) {
				swingticks = 340;
				requireswing = false;
			}
			else {
				swingticks += 1;
			}
			if (swingticks <= 339) swingticks = 340;
		}
	}

	//if (slowSwing && !g_Hooks.isThirdPerson) { float* speedAdr = reinterpret_cast<float*>(reinterpret_cast<__int64>(player) + 0x79C); *speedAdr = swingticks; }
	if (noswing && !g_Hooks.isThirdPerson) { float* speedAdr = reinterpret_cast<float*>(reinterpret_cast<__int64>(player) + 0x79C); *speedAdr = 0; }

	if (mode.getSelectedValue() == 5 && shouldBlock) { float* speedAdr = reinterpret_cast<float*>(reinterpret_cast<__int64>(player) + 0x79C); *speedAdr = 29; }

	if (isAttacking && shouldBlock && mode.getSelectedValue() == 2 && !g_Hooks.isThirdPerson) { float* speedAdr = reinterpret_cast<float*>(reinterpret_cast<__int64>(player) + 0x79C); *speedAdr = swingSpeed; }
}

void Animations::onDisable() {
	if (type.getSelectedValue() == 1) Utils::patchBytes((unsigned char*)((uintptr_t)SmoothSwing), (unsigned char*)"\x0F\x84\x95\x02\x00\x00", 6);
	if (type.getSelectedValue() == 2) Utils::patchBytes((BYTE*)((uintptr_t)TapAddress), (BYTE*)"\xF3\x0F\x51\xF0", 4);
	if (type.getSelectedValue() == 3) {
		Utils::patchBytes((unsigned char*)((uintptr_t)SmoothSwing), (unsigned char*)"\x0F\x84\x95\x02\x00\x00", 6);
		Utils::patchBytes((BYTE*)((uintptr_t)TapAddress), (BYTE*)"\xF3\x0F\x51\xF0", 4);
	}
}