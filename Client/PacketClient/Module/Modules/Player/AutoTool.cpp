#include "AutoTool.h"
#include "../pch.h"

AutoTool::AutoTool() : IModule(0, Category::PLAYER, "Automatically selects the best tool.") {
	registerBoolSetting("Weapons", &weapon, weapon);
	registerBoolSetting("Tools", &tools, tools);
}

const char* AutoTool::getModuleName() {
	return ("AutoTool");
}

void AutoTool::onEnable() {
	hasClicked = false;
	prevslot = g_Data.getLocalPlayer()->getSupplies()->selectedHotbarSlot;
}
bool attacknow = false;
int attacktime = 0;
int prevslotWeapon = 0;
void AutoTool::onAttack(C_Entity* attackedEnt) {
	auto player = g_Data.getLocalPlayer();
	if (!weapon) return;
	if (player == nullptr) return;
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;
	int slot = -1;
	int damage = 0;
	for (int n = 0; n < 9; n++) {
		C_ItemStack* stack = inv->getItemStack(n);
		if (stack->item != nullptr) {
			float currentDamage = stack->getAttackingDamageWithEnchants();
			if (currentDamage > damage && currentDamage >= 1) {
				damage = currentDamage;
				slot = n;
			}
		}
	}
	if (slot != -1) {
		if (supplies->selectedHotbarSlot != slot) {
			prevslotWeapon = supplies->selectedHotbarSlot;
			supplies->selectedHotbarSlot = slot;
		}
		attacknow = true;
		attacktime = 0;
	}
}
void AutoTool::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;
	if (attacknow) attacktime++;
	if (attacktime >= 3) {
		attacktime = 0;
		attacknow = false;
		supplies->selectedHotbarSlot = prevslotWeapon;
	}
	if (!tools) return;
	if (g_Data.getLocalPlayer()->region->getBlock(player->pointingStruct->block)->blockLegacy->blockId == 7 || player->pointingStruct->block == vec3_t(0, 0, 0)) {
		if (hasClicked) {
			supplies->selectedHotbarSlot = prevslot;
			hasClicked = false;
		}
		return;
	}
	if (g_Data.canUseMoveKeys() && !moduleMgr->getModule<ClickGUIMod>()->isEnabled()) {
		float damage = 0;
		if (GameData::isLeftClickDown()) {
			if (!hasClicked) {
				prevslot = supplies->selectedHotbarSlot;
				hasClicked = true;
			}
			else {
				int slot = -1;
				for (int n = 0; n < 9; n++) {
					C_ItemStack* stack = inv->getItemStack(n);
					if (stack->item != nullptr) {
						float currentDamage = stack->getItem()->getAttackDamage() + stack->getItem()->getDestroySpeed(*stack, *g_Data.getLocalPlayer()->region->getBlock(player->pointingStruct->block));
						bool IsUseful = stack->getItem()->getDestroySpeed(*stack, *g_Data.getLocalPlayer()->region->getBlock(player->pointingStruct->block)) <= 32767;
						if (currentDamage > damage && IsUseful && currentDamage >= 1) {
							damage = currentDamage;
							slot = n;
						}
					}
				}
				if (slot != -1) supplies->selectedHotbarSlot = slot;
			}
		}
		else if (hasClicked) {
			supplies->selectedHotbarSlot = prevslot;
			hasClicked = false;
		}
	}
}
