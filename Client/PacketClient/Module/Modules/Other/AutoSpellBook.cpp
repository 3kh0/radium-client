#include "AutoSpellBook.h"
#include "../pch.h"

using namespace std;
AutoSpellBook::AutoSpellBook() : IModule(0, Category::OTHER, "Automatically use spellbook when low on health") {
}

const char* AutoSpellBook::getModuleName() {
	return ("AutoSpellBook");
}

vector<string> spellbookname = {
	"enchanted_book"
};



void AutoSpellBook::onPlayerTick(C_Player* plr) {
	if (plr == nullptr) return;

	auto selectedItem = plr->getSelectedItem();

}

void AutoSpellBook::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	auto selectedItem = player->getSelectedItem();

	if (g_Data.canUseMoveKeys() && !moduleMgr->getModule<ClickGUIMod>()->isEnabled()) {

		C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
		C_Inventory* inv = supplies->inventory;
		auto prevSlot = supplies->selectedHotbarSlot;

		if (player->getHealth() < 13) {
			for (int n = 0; n < 36; n++) {
				C_ItemStack* stack = inv->getItemStack(n);
				if (stack->item != nullptr) {
					string ItemName2 = stack->getItem()->name.getText();
					if (ItemName2.find("enchanted_book") != string::npos) {
						if (prevSlot != n) {
							supplies->selectedHotbarSlot = n;
							gm->useItem(*stack);
							supplies->selectedHotbarSlot = prevSlot;
							return;
						}
					}
				}
			}
		}
	}
}