#include "AutoHive.h"
#include "../../../../Utils/TimerUtil.h"
#include "../../../../SDK/CAttribute.h"
#include "../pch.h"

int Odelay;

using namespace std;
AutoHive::AutoHive() : IModule(0, Category::OTHER, "Automates things on The Hive") {
	registerBoolSetting("AutoLootBox", &autoLootBox, autoLootBox);
	registerBoolSetting("AutoQueue", &autoQueue, autoQueue);
	registerBoolSetting("AutoSnow", &autosb, autosb);
	registerBoolSetting("AutoBridgeWin", &autoBridgeWin, autoBridgeWin);
}

const char* AutoHive::getRawModuleName() {
	return "AutoHive";
}

const char* AutoHive::getModuleName() {
	return "AutoHive";
}

static vector<C_Entity*> entityList;
void findEntity_AH(C_Entity* currentEntity, bool isRegularEntity) {
	static auto autoHive = moduleMgr->getModule<AutoHive>();
	static auto killaura = moduleMgr->getModule<Killaura>();

	if (g_Data.canUseMoveKeys() && g_Data.getLocalPlayer() != nullptr) {
		if (currentEntity == nullptr) return;
		if (currentEntity == g_Data.getLocalPlayer()) return;
		if (!g_Data.getLocalPlayer()->canAttack(currentEntity, false)) return;
		if (!g_Data.getLocalPlayer()->isAlive()) return;
		if (!currentEntity->isAlive()) return;

		if (autoHive->autoBridgeWin) {
			if (currentEntity->getEntityTypeId() == 80 || currentEntity->getEntityTypeId() == 69) return;
			if (!TargetUtil::isValidTarget(currentEntity)) return;
			if (currentEntity->getEntityTypeId() == 51 || currentEntity->getEntityTypeId() == 1677999) return;
			entityList.push_back(currentEntity);
		}
		else {
			if (currentEntity->getEntityTypeId() != 256) return; // LootBoxes
			if (currentEntity->getNameTag()->getTextLength() >= 1) return;
			if (currentEntity->width <= 0.1f || currentEntity->height <= 0.1f) return;
			if (currentEntity->width >= 0.9f || currentEntity->height >= 0.9f) return;
			float dist = (*currentEntity->getPos()).dist(*g_Data.getLocalPlayer()->getPos());
			if (dist < 5 && killaura->targetListEmpty) entityList.push_back(currentEntity);
		}
	}
}

static vector<C_Entity*> entityList2;
void findEntity_AH2(C_Entity* currentEntity, bool isRegularEntity) {
	static auto autoHive = moduleMgr->getModule<AutoHive>();
	static auto killaura = moduleMgr->getModule<Killaura>();

	if (autoHive->autosb) {

		if (currentEntity == nullptr) return;
		if (currentEntity == g_Data.getLocalPlayer()) return;
		if (!g_Data.getLocalPlayer()->canAttack(currentEntity, false)) return;
		if (!g_Data.getLocalPlayer()->isAlive()) return;
		if (!currentEntity->isAlive()) return;
		if (currentEntity->getEntityTypeId() == 80 || currentEntity->getEntityTypeId() == 69) return;

		if (killaura->mobs) {
			if (currentEntity->getNameTag()->getTextLength() <= 1 && currentEntity->getEntityTypeId() == 63) return;
			if (currentEntity->width <= 0.01f || currentEntity->height <= 0.01f) return;
			if (currentEntity->getEntityTypeId() == 64) return;
		}
		else {
			if (!TargetUtil::isValidTarget(currentEntity)) return;
			if (currentEntity->getEntityTypeId() == 51 || currentEntity->getEntityTypeId() == 1677999) return;
		}

		float dist = (*currentEntity->getPos()).dist(*g_Data.getLocalPlayer()->getPos());
		if (dist < killaura->range) entityList2.push_back(currentEntity);
	}
}

struct CompareTargetEnArray {
	bool operator()(C_Entity* lhs, C_Entity* rhs) {
		C_LocalPlayer* localPlayer = g_Data.getLocalPlayer();
		return (*lhs->getPos()).dist(*localPlayer->getPos()) < (*rhs->getPos()).dist(*localPlayer->getPos());
	}
};

void AutoHive::onEnable() {
	entityList.clear();
	entityList2.clear();
	if (autoBridgeWin) doLerp = true;
	sendcommand = false;
	Odelay = 0;
}

vector<string> snowball = {
	"snowball"
};


void AutoHive::onTick(C_GameMode* gm) {

	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();

	auto killaura = moduleMgr->getModule<Killaura>();
	auto player = g_Data.getLocalPlayer();
	auto timerUtil = new TimerUtil();
	if (player == nullptr) return;

	entityList.clear();
	g_Data.forEachEntity(findEntity_AH);
	entityList2.clear();
	g_Data.forEachEntity(findEntity_AH2);
	lootBoxListEmpty = entityList.empty();

	C_Inventory* inv = supplies->inventory;
	auto prevSlot = supplies->selectedHotbarSlot;

	if (autosb && !entityList2.empty()) {
		for (int n = 0; n < 36; n++) {
			C_ItemStack* stack = inv->getItemStack(n);
			if (stack->item != nullptr) {
				string ItemName2 = stack->getItem()->name.getText();
				if (ItemName2.find("snowball") != string::npos) {
					if (prevSlot != n) {
						supplies->selectedHotbarSlot = n;
						Odelay++;
						if (Odelay >= 3) {
							gm->useItem(*stack);
							Odelay = 0;
						}
						supplies->selectedHotbarSlot = prevSlot;
						return;
					}
				}
			}
		}
	}

	//AutoQueue
	if (sendcommand) {
		CommandRequestPacket commandPacket;
		commandPacket.payload = string("/me");
		commandPacket.one = 1;
		commandPacket.two = 2;
		g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&commandPacket);
		auto notififcation = g_Data.addNotification("AutoQueue:", "Queueing"); notififcation->duration = 5;
		sendcommand = false;
	}

	static int delay = 0;
	vec3_t* pos = gm->player->getPos();
	if (g_Data.canUseMoveKeys() && killaura->targetListEmpty && !autoBridgeWin) {
		delay++;
		if (delay >= 5) {
			sort(entityList.begin(), entityList.end(), CompareTargetEnArray());
			for (auto& i : entityList) {
				gm->attack(entityList[0]);
			}
			delay = 0;
		}
	}
	else lootBoxListEmpty = true;
}

void AutoHive::onPlayerTick(C_Player* plr) {
	auto killaura = moduleMgr->getModule<Killaura>();
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	auto selectedItem = player->getSelectedItem();

	if (autosb) {
		auto selectedItem = plr->getSelectedItem();
	}

	if (g_Data.canUseMoveKeys() && killaura->targetListEmpty && !autoBridgeWin) {
		vec2_t angle;
		sort(entityList.begin(), entityList.end(), CompareTargetEnArray());
		for (auto& i : entityList) {
			angle = g_Data.getLocalPlayer()->getPos()->CalcAngle(*entityList[0]->getPos());
			vec2_t pos = g_Data.getLocalPlayer()->getPos()->CalcAngle(*i->getPos());

			if (animYaw > angle.y) animYaw -= ((animYaw - angle.y) / 10);
			else if (animYaw < angle.y) animYaw += ((angle.y - animYaw) / 10);

			plr->yawUnused1 = animYaw;
			plr->bodyYaw = animYaw;
			plr->pitch = angle.x;
		}
	}

	if (autoBridgeWin) {
		sort(entityList.begin(), entityList.end(), CompareTargetEnArray());
		for (auto& i : entityList) {
			if (doLerp) {
				moduleMgr->getModule<Blink>()->setEnabled(true);
				vec3_t tpPos = *entityList[0]->getPos(); tpPos.y -= 15;

				if (player->getPos()->x < 0) tpPos.x += 12; else tpPos.x -= 12;

				float dist = player->getPos()->dist(tpPos);
				player->lerpTo(tpPos, vec2_t(1, 1), (int)fmax((int)dist * 0.1, 30));
				doLerp = false;
			}

			vec3_t tpPos = *entityList[0]->getPos(); tpPos.y -= 15;
			if (player->getPos()->y <= tpPos.y) moduleMgr->getModule<Blink>()->setEnabled(false);
		}
	}
}

void AutoHive::onSendPacket(C_Packet* packet) {
}

void AutoHive::onDisable() {
	moduleMgr->getModule<Blink>()->setEnabled(false);
	lootBoxListEmpty = true;
}
