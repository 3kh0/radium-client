#include "Killaura.h"
#include "../pch.h"

int slot;

using namespace std;
Killaura::Killaura() : IModule(0, Category::COMBAT, "Attacks entities") {
	registerEnumSetting("Rotations", &rotations, 0);
	rotations.addEntry("Normal", 0);
	rotations.addEntry("Smooth", 1);
	rotations.addEntry("Hive", 2);
	rotations.addEntry("No360", 3);
	rotations.addEntry("Silent", 4);
	rotations.addEntry("None", 5);
	rotations.addEntry("Old", 6);
	rotations.addEntry("DesyncRot", 7);
	rotations.addEntry("Exhibition", 8);
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Multi", 0);
	mode.addEntry("Switch", 1);
	mode.addEntry("Single", 2);
	registerEnumSetting("AutoSword", &autosword, 0);
	autosword.addEntry("None", 0);
	autosword.addEntry("Switch", 1);
	autosword.addEntry("Spoof", 2);
	registerBoolSetting("Strafe", &strafe, strafe);
	registerBoolSetting("Click", &click, click);
	registerBoolSetting("InstaRot", &instarot, instarot);
	registerBoolSetting("Hold", &hold, hold);
	registerBoolSetting("Visualize", &visualize, visualize);
	registerBoolSetting("VisualRange", &visualrange, visualrange);
	registerFloatSetting("Smoothing", &smoothing, smoothing, 1.f, 80.f);
	registerFloatSetting("Random", &random, random, 0.f, 15.f);
	registerFloatSetting("Range", &range, range, 3.f, 8.f);
	//registerMinMaxSetting("APS", &maxAPS, maxAPS, 1, 20);
	registerIntSetting("MaxAPS", &maxAPS, maxAPS, 1, 20);
	registerIntSetting("MinAPS", &minAPS, minAPS, 1, 20);
}

const char* Killaura::getRawModuleName() {
	return "Killaura";
}

const char* Killaura::getModuleName() {
	switch (mode.getSelectedValue()) {
	case 0: name = string("Killaura ") + string(GRAY) + string("Multi"); break;
	case 1: name = string("Killaura ") + string(GRAY) + string("Switch"); break;
	case 2: name = string("Killaura ") + string(GRAY) + string("Single"); break;
	}
	return name.c_str();
}

static vector<C_Entity*> targetList;
void findEntity(C_Entity* currentEntity, bool isRegularEntity) {
	static auto killaura = moduleMgr->getModule<Killaura>();

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
	if (dist < killaura->range) targetList.push_back(currentEntity);
}

struct CompareTargetEnArray {
	bool operator()(C_Entity* lhs, C_Entity* rhs) {
		C_LocalPlayer* localPlayer = g_Data.getLocalPlayer();
		return (*lhs->getPos()).dist(*localPlayer->getPos()) < (*rhs->getPos()).dist(*localPlayer->getPos());
	}
};

void Killaura::onEnable() {
	C_InventoryTransactionManager* manager = g_Data.getLocalPlayer()->getTransactionManager();
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	targetListEmpty = true;
	preventflagrot = false;
	targetList.clear();

	slot = supplies->selectedHotbarSlot;
}

void Killaura::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (player->canFly) {
		setEnabled(false);
		return;
	}

	if (moduleMgr->getModule<Regen>()->breaknow == true) {
		animYaw = player->yawUnused1;
		animPitch = player->pitch;
		return;
	}

	PointingStruct* pointing = g_Data.getLocalPlayer()->pointingStruct;
	auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	if (clickGUI->isEnabled()) targetListEmpty = true;
	targetListEmpty = targetList.empty();
	C_InventoryTransactionManager* manager = g_Data.getLocalPlayer()->getTransactionManager();
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;

	targetList.clear();
	g_Data.forEachEntity(findEntity);

	//Prevent flag
	//if (moduleMgr->getModule<Regen>()->isEnabled() && moduleMgr->getModule<Regen>()->enabledticks < 3 && !moduleMgr->getModule<Regen>()->canattack) return;
	if (rotations.getSelectedValue() == 2 || rotations.getSelectedValue() == 7) {
		if (preventflagrot) {
			//clientMessageF("PreventFlag");
			return;
		}
	}

	if (autosword.getSelectedValue() == 2 && !targetList.empty()) slot = supplies->selectedHotbarSlot;

	//Strafe Disabler
	C_MovePlayerPacket mp;
	if (moduleMgr->getModule<Disabler>()->hivetest == true && !targetList.empty()) {
		vec3_t enemypos = *targetList[0]->getPos();
		enemypos.y = player->getPos()->y;
		float distance = (enemypos).dist(*g_Data.getLocalPlayer()->getPos());
		if (distance < 1) {
			mp.Position.x = targetList[0]->currentPos.x;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&mp);
			mp.Position.z = targetList[0]->currentPos.z;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&mp);
			//clientMessageF("Packet");
		}
	}

	//AutoSword

	float SwordDamage = 0;

	int Sword = -1;

	if (!targetList.empty() && autosword.getSelectedValue() > 0) {
		for (int n = 0; n < 36; n++) {
			C_ItemStack* stack = inv->getItemStack(n);
			if (stack->item != nullptr) {
				float currentDamage = stack->getAttackingDamageWithEnchants();
				//stack->setShowPickUp(true);
				float blockCount = stack->count;
				if (/*stack->getItem()->isSword() && */currentDamage > SwordDamage) {
					SwordDamage = currentDamage;
					Sword = n;
				}
			}
		}

		if (Sword != -1)
			supplies->selectedHotbarSlot = Sword;
	}

	if (g_Data.canUseMoveKeys() && !targetList.empty()) {
		if (TimerUtil::hasTimedElapsed((1000 / APS), true)) {
			if (hold && !g_Data.isLeftClickDown())
				return;

			if (!moduleMgr->getModule<NoSwing>()->isEnabled())
				player->swing();

			for (auto& i : targetList) {
				if (!targetListEmpty) atk = true;
				if (atk) {
					vec2_t angle = g_Data.getLocalPlayer()->getPos()->CalcAngle(*targetList[0]->getPos());
					switch (rotations.getSelectedValue()) {
					case 0: // Normal
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						player->bodyYaw = angle.y;
						break;
					case 1: // Smooth
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						break;
					case 2: // Hive
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						player->bodyYaw = angle.y;
						break;
					case 3: // Hive+
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						player->bodyYaw = angle.y;
						break;
					case 6: //Old
						player->yawUnused2 = angle.y;
						player->pitch2 = angle.x;
						player->bodyYaw = angle.y;
						break;
					case 7: // Desync
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						player->bodyYaw = angle.y;
						break;
					case 8: //Exhi
						player->yawUnused1 = angle.y;
						player->pitch = 69;
						player->bodyYaw = angle.y;
						break;
					}

					// Strafe
					if (/*mode.getSelectedValue() <= 2 &&*/ strafe) {
						player->setRot(angle);
					}
					if (mode.getSelectedValue() == 0) gm->attack(i);
					else gm->attack(targetList[0]);
					atk = false;
				}
			}
		}

		for (auto& i : targetList) {
			delay++;
			if (click && !targetList.empty()) {
				if (delay >= 0) {
					g_Data.leftclickCount++;
					if (pointing->hasEntity()) gm->attack(pointing->getEntity());
				}
			}
		}
		//AutoSword Spoof
		if (autosword.getSelectedValue() == 2) supplies->selectedHotbarSlot = slot;
	}
	else targetListEmpty = true;
}

void Killaura::onPlayerTick(C_Player* plr) {
	if (plr == nullptr) return;
	if (moduleMgr->getModule<Regen>()->breaknow == true) return;
	if (!instarot)
	{
		if (targetList.empty() && rotations.getSelectedValue() == 2) {
			animYaw = plr->yawUnused1;
			animPitch = plr->pitch;
		}

		if (targetList.empty() && rotations.getSelectedValue() == 7) {
			animYaw = plr->yawUnused1;
			animPitch = plr->pitch;
		}
	}

	if (g_Data.canUseMoveKeys() && !targetList.empty()) {
		APS = randomFloat(maxAPS, minAPS);
		if (maxAPS < minAPS) maxAPS = minAPS;
		if (hold && !g_Data.isLeftClickDown())
			return;

		xRandom = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / random));
		yRandom = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / random));
		if (random >= 0.f) { yRandom += 1.75f; xRandom -= 3.5f; }

		sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		for (auto& i : targetList) {
			vec2_t angle = g_Data.getLocalPlayer()->getPos()->CalcAngle(*targetList[0]->getPos());
			vec2_t pos = g_Data.getLocalPlayer()->getPos()->CalcAngle(*i->getPos());


			if (rotations.getSelectedValue() == 3) {
				if (100 < animYaw - angle.y || 100 < angle.y - animYaw) {
					if (animYaw > angle.y) animYaw += ((angle.y - animYaw) / smoothing);
					else if (animYaw < angle.y) animYaw -= ((animYaw - angle.y) / smoothing);
				}
			}

			if (rotations.getSelectedValue() == 2 || rotations.getSelectedValue() == 7) {
				if (50 < animYaw - angle.y || 50 < angle.y - animYaw) {
					preventflagrot = true;
				}
				else
				{
					preventflagrot = false;
				}
			}

			if (animYaw > angle.y) animYaw -= ((animYaw - angle.y) / smoothing);
			else if (animYaw < angle.y) animYaw += ((angle.y - animYaw) / smoothing);

			if (animPitch > angle.x) animPitch -= ((animPitch - angle.x) / smoothing);
			else if (animPitch < angle.x) animPitch += ((angle.x - animPitch) / smoothing);
			if (!instarot)
			{
				switch (rotations.getSelectedValue()) {
				case 0: // Normal
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = angle.x + yRandom;
					plr->bodyYaw = animYaw + xRandom;
					break;
				case 1: // Smooth
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = angle.x + yRandom;
					break;
				case 2: // Hive
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = animPitch + yRandom;
					plr->bodyYaw = animYaw + xRandom;
					break;
				case 3: // Hive+
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = animPitch + yRandom;
					plr->bodyYaw = animYaw + xRandom;
					break;
				case 6: //Old
					plr->yawUnused2 = animYaw + xRandom;
					plr->pitch2 = angle.x + yRandom;
					plr->bodyYaw = animYaw + xRandom;
					break;
				case 7: // Desync
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = animPitch + yRandom;
					plr->bodyYaw = angle.y;
					break;
				case 8: //Exhi
					plr->yawUnused1 = animYaw + xRandom;
					plr->pitch = 69;
					plr->bodyYaw = animYaw + xRandom;
					break;
				}

				// Strafe
				if (/*mode.getSelectedValue() <= 2 &&*/ strafe) {
					plr->setRot(angle);
				}
			}
		}
	}
}

void Killaura::onSendPacket(C_Packet* packet) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	if (moduleMgr->getModule<Regen>()->breaknow == true) return;

	if (g_Data.canUseMoveKeys()) {
		if (hold && !g_Data.isLeftClickDown())
			return;

		sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		if (packet->isInstanceOf<C_MovePlayerPacket>() && rotations.getSelectedValue() != 3 && !targetList.empty()) {
			vec2_t angle = g_Data.getLocalPlayer()->getPos()->CalcAngle(targetList[0]->eyePos0);
			auto* movePacket = reinterpret_cast<C_MovePlayerPacket*>(packet);

			if (rotations.getSelectedValue() == 2 || rotations.getSelectedValue() == 3) {
				movePacket->pitch = animPitch + yRandom;
				movePacket->headYaw = animYaw + xRandom;
				movePacket->yaw = animYaw + xRandom;
			}
			else if (rotations.getSelectedValue() == 7) {
				movePacket->pitch = animPitch + yRandom;
				movePacket->headYaw = animYaw + xRandom;
				movePacket->yaw = angle.y + xRandom;
			}
			else
			{
				movePacket->pitch = angle.x + yRandom;
				movePacket->headYaw = animYaw + xRandom;
				movePacket->yaw = animYaw + xRandom;
			}
		}
		if (packet->isInstanceOf<PlayerAuthInputPacket>() && rotations.getSelectedValue() != 3 && !targetList.empty()) {
			vec2_t angle = g_Data.getLocalPlayer()->getPos()->CalcAngle(targetList[0]->eyePos0);
			auto* authPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);

			if (rotations.getSelectedValue() == 2 || rotations.getSelectedValue() == 3) {
				authPacket->pos.x = animPitch + yRandom;
				authPacket->pos.y = animYaw + xRandom;
			}
			else if (rotations.getSelectedValue() == 7) {
				authPacket->pos.x = animPitch + yRandom;
				authPacket->pos.y = angle.y + xRandom;
			}
			else
			{
				authPacket->pos.x = angle.x + yRandom;
				authPacket->pos.y = animYaw + xRandom;
			}
		}
	}
}

void Killaura::onLevelRender() {
	auto targetStrafe = moduleMgr->getModule<TargetStrafe>();
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	//VisualRange
	static float n2 = 0;
	static float anim2 = 0;
	if (g_Data.canUseMoveKeys()) {
		if (!visualrange)
			return;
		if (mode.getSelectedValue() == 2) sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		anim2++;
		DrawUtils::setColor(1, 1, 1, 0.9f);

		vec3_t permutations2[56];
		for (int i = 0; i < 56; i++) {
			permutations2[i] = { sinf((i * 20.f) / (180 / PI)), 0.f, cosf((i * 20.f) / (180 / PI)) };
		}
		vec3_t permutations22[56];
		n2++;
		if (n2 == 360)
			n2 = 0;
		for (int i = 0; i < 56; i++) {
			permutations22[i] = { sinf((i * 10.f) / (180 / PI)), sinf((i * 100.f) / (180 / PI)) ,cosf((i * 10.f) / (180 / PI)) };
		}

		//const float coolAnim2 = 0.9f + 0.9f * sin((anim2 / 60) * PI * 1);

		vec3_t* start2 = player->getPosOld();
		vec3_t* end2 = player->getPos();

		auto te2 = DrawUtils::getLerpTime();
		vec3_t pos2 = start2->lerp(end2, te2);

		auto yPos2 = pos2.y;
		//yPos2 -= 1.6f;
		yPos2 = player->getPos()->y - 1.f;
		//yPos2 += coolAnim2;

		vector<vec3_t> posList2;
		posList2.reserve(range * 56);
		for (auto& perm2 : permutations2) {
			vec3_t curPos(pos2.x, yPos2, pos2.z);
			posList2.push_back(curPos.add(perm2));
		}
		DrawUtils::drawLineStrip3D(posList2, 5);
	}

	if (player == nullptr || targetStrafe->isEnabled() || !visualize) return;
	if (moduleMgr->getModule<Regen>()->breaknow == true) return;

	static float n = 0;
	static float anim = 0;
	if (g_Data.canUseMoveKeys() && !targetList.empty()) {
		if (hold && !g_Data.isLeftClickDown())
			return;
		if (mode.getSelectedValue() == 2) sort(targetList.begin(), targetList.end(), CompareTargetEnArray());
		if (!targetList.empty()) {
			anim++;
			DrawUtils::setColor(1, 1, 1, 0.9f);

			vec3_t permutations[56];
			for (int i = 0; i < 56; i++) {
				permutations[i] = { sinf((i * 10.f) / (180 / PI)), 0.f, cosf((i * 10.f) / (180 / PI)) };
			}
			vec3_t permutations2[56];
			n++;
			if (n == 360)
				n = 0;
			for (int i = 0; i < 56; i++) {
				permutations2[i] = { sinf((i * 10.f) / (180 / PI)), sinf((i * 10.f) / (180 / PI)) ,cosf((i * 10.f) / (180 / PI)) };
			}

			const float coolAnim = 0.9f + 0.9f * sin((anim / 60) * PI * 1);

			vec3_t* start = targetList[0]->getPosOld();
			vec3_t* end = targetList[0]->getPos();

			auto te = DrawUtils::getLerpTime();
			vec3_t pos = start->lerp(end, te);

			auto yPos = pos.y;
			yPos -= 1.6f;
			yPos += coolAnim;

			vector<vec3_t> posList;
			posList.reserve(56);
			for (auto& perm : permutations) {
				vec3_t curPos(pos.x, yPos, pos.z);
				posList.push_back(curPos.add(perm));
			}
			DrawUtils::drawLineStrip3D(posList, 5);

			//Draw Sims Esp
			/*vector<vec3_t> posList2;
			posList2.clear();
			// Iterate through phi, theta then convert r,theta,phi to XYZ
			for (double phi = 0.; phi < 2 * PI; phi += PI / 100.) // Azimuth [0, 2PI]
			{
				for (double theta = 0.; theta < PI; theta += PI / 100.) // Elevation [0, PI]
				{
					vec3_t point;
					point.x = 0.2 * cos(phi) * sin(theta);
					point.y = 0.2 * sin(phi) * sin(theta);
					point.z = 0.2 * cos(theta);
					vec3_t curPos = targetList[0]->eyePos0;
					curPos.y += 0.5f;
					posList2.push_back(curPos.add(point));
				}
			}
			DrawUtils::setColor(0.1f,0.9f,0.1f,255);
			DrawUtils::drawLineStrip3D(posList2, 4);*/
		}
	}
}

void Killaura::onDisable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	targetListEmpty = true;
	targetList.clear();
}
