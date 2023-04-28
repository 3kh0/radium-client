#include "AutoAim.h"
#include "../pch.h"

using namespace std;
AutoAim::AutoAim() : IModule(0, Category::COMBAT, "Automatically aims at the nearest entity") {
	registerBoolSetting("Vertical", &vertical, vertical);
	registerBoolSetting("Hold", &click, click);
	registerBoolSetting("AdvancedLook", &advanced, advanced);
	registerBoolSetting("SlowY", &slowy, slowy);
	registerFloatSetting("Range", &range, range, 3.f, 8.f);
	registerFloatSetting("Angle", &angleAmount, angleAmount, 20.f, 180.f);
}

const char* AutoAim::getModuleName() {
	return ("AutoAim");
}

static vector<C_Entity*> targetList;
void findEntity_AutoAim(C_Entity* currentEntity, bool isRegularEntity) {
	static auto autoAim = moduleMgr->getModule<AutoAim>();

	if (currentEntity == nullptr)
		return;

	if (currentEntity == g_Data.getLocalPlayer())
		return;

	if (!g_Data.getLocalPlayer()->canAttack(currentEntity, false))
		return;

	if (!g_Data.getLocalPlayer()->isAlive())
		return;

	if (!currentEntity->isAlive())
		return;

	if (currentEntity->getEntityTypeId() == 69)  // XP
		return;

	if (currentEntity->getEntityTypeId() == 80)  // Arrows
		return;

	if (!TargetUtil::isValidTarget(currentEntity))
		return;

	if (currentEntity->getEntityTypeId() == 1677999)  // Villager
		return;

	if (currentEntity->getEntityTypeId() == 51)  // NPC
		return;

	float dist = (*currentEntity->getPos()).dist(*g_Data.getLocalPlayer()->getPos());

	if (dist < autoAim->range) {
		targetList.push_back(currentEntity);
	}
}

struct CompareTargetEnArray {
	bool operator()(C_Entity* lhs, C_Entity* rhs) {
		auto player = g_Data.getLocalPlayer();
		return (int)((*lhs->getPos()).dist(*player->getPos())) < (int)((*rhs->getPos()).dist(*player->getPos()));
	}
};

void AutoAim::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	targetList.clear();
}

void AutoAim::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	targetList.clear();
	g_Data.forEachEntity(findEntity_AutoAim);
}

void AutoAim::onPlayerTick(C_Player* plr) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (click && !g_Data.isLeftClickDown())
		return;

	if (g_Data.canUseMoveKeys() && !moduleMgr->getModule<ClickGUIMod>()->isEnabled()) {
		sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		if (!targetList.empty()) {
			vec3_t advancedlook = *targetList[0]->getPos();
			if(player->getPos()->y + 1.2 < targetList[0]->getPos()->y && advanced){
				advancedlook.y -= 1.f;
			}
			else
			{
				//nope lmao
			}
			vec2_t angle = player->getPos()->CalcAngle(advancedlook);
			vec3_t origin = g_Data.getClientInstance()->levelRenderer->getOrigin();
			vec2_t appl = angle.sub(player->viewAngles).normAngles();

			if (AnimYaw > angle.y) AnimYaw -= ((AnimYaw - angle.y) / 20);
			else if (AnimYaw < angle.y) AnimYaw += ((angle.y - AnimYaw) / 20);
			angle.y = AnimYaw;

			for (int i = 0; i < 10; i++) {
					appl.x = -appl.x;
					if ((appl.x < angleAmount && appl.x > -angleAmount) && (appl.y < angleAmount && appl.y > -angleAmount)) {
						{
							if (slowy) {
								appl.x /= (21);
							}
							else
							{
								appl.x /= (7);
							}
							appl.y /= (7);
						}
						if (appl.x >= 0.01 || appl.x <= -0.01) appl.div(abs(appl.x));
						if (appl.y >= 0.01 || appl.y <= -0.01) appl.div(abs(appl.y));
						if (!vertical)
							appl.x = 0;
						player->applyTurnDelta(&appl);
					}
			}
		}
	}
}

void AutoAim::onDisable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
}
