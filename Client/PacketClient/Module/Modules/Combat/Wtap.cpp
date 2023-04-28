#include "Wtap.h"
#include "../pch.h"

using namespace std;
Wtap::Wtap() : IModule(0, Category::COMBAT, "Automatically wtap") {
	registerFloatSetting("Range", &range, range, 3.f, 8.f);
	registerIntSetting("Delay", &delay, delay, 1, 20);
	registerBoolSetting("Sprint", &sprint, sprint);
	registerBoolSetting("Hold", &hold, hold);
}

const char* Wtap::getModuleName() {
	return ("Wtap");
}

static vector<C_Entity*> targetList;
void findEntity_Wtap(C_Entity* currentEntity, bool isRegularEntity) {
	static auto aimbot = moduleMgr->getModule<Wtap>();

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

	if (dist < aimbot->range) {
		targetList.push_back(currentEntity);
	}
}

struct CompareTargetEnArray {
	bool operator()(C_Entity* lhs, C_Entity* rhs) {
		auto player = g_Data.getLocalPlayer();
		return (int)((*lhs->getPos()).dist(*player->getPos())) < (int)((*rhs->getPos()).dist(*player->getPos()));
	}
};

void Wtap::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	targetList.clear();

	delay2 = 0;
}

void Wtap::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	delay2++;
	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	bool pressw = GameData::isKeyDown(*input->forwardKey);
	sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		if (delay2 > delay - 1)
		{
			if (hold && !g_Data.isLeftClickDown()) return;
			delay2 = 0;
			if (!targetList.empty()) {
				g_Data.getClientInstance()->getMoveTurnInput()->forward = false;
			}
		}
		else if (pressw && delay2 > delay / 1.5)
		{
			g_Data.getClientInstance()->getMoveTurnInput()->forward = true;
			if (sprint) {
				player->setSprinting(true);
			}
		}

	targetList.clear();
	g_Data.forEachEntity(findEntity_Wtap);
}

void Wtap::onPlayerTick(C_Player* plr) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	/*if (click && !g_Data.isLeftClickDown())
		return;

	if (g_Data.canUseMoveKeys() && !moduleMgr->getModule<ClickGUIMod>()->isEnabled()) {
		sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		if (!targetList.empty()) {
			vec2_t angle = player->getPos()->CalcAngle(*targetList[0]->getPos());
			vec3_t origin = g_Data.getClientInstance()->levelRenderer->getOrigin();
			vec2_t appl = angle.sub(player->viewAngles).normAngles();

			if (lock) {
				player->setRot(angle);
				player->resetRot();
			}
			else {
				appl.x = -appl.x;
				if ((appl.x < angleAmount && appl.x > -angleAmount) && (appl.y < angleAmount && appl.y > -angleAmount)) {
					appl.x *= (100.0f - speed);
					appl.y *= (100.0f - speed);
					if (appl.x >= 1 || appl.x <= -1) appl.div(abs(appl.x));
					if (appl.y >= 1 || appl.y <= -1) appl.div(abs(appl.y));
					if (!vertical)
						appl.x = 0;
					player->applyTurnDelta(&appl);
				}
			}
		}
	}*/
}

void Wtap::onDisable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
}
