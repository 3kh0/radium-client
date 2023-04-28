#include "Regen.h"
#include "../../../../SDK/CAttribute.h"
#include "../pch.h"

using namespace std;
Regen::Regen() : IModule(0, Category::COMBAT, "Regenerates your health") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Hive", 0);
	mode.addEntry("HiveTest", 1);
	registerFloatSetting("Range", &range, range, 1, 7);
	registerIntSetting("ContinueDelay", &delay, delay, 0, 20);
	registerIntSetting("MineDelay", &minedelay, minedelay, 0, 30);
	registerBoolSetting("Visual", &visual, visual);
	registerBoolSetting("HealthVisual", &healthvisual, healthvisual);
}

const char* Regen::getRawModuleName() {
	return "Regen";
}

const char* Regen::getModuleName() {
	if (mode.getSelectedValue() == 0) name = string("Regen ") + string(GRAY) + string("Hive");
	if (mode.getSelectedValue() == 1) name = string("Regen ") + string(GRAY) + string("HiveTest");
	return name.c_str();
}

bool Regen::selectPickaxe() {
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	C_Inventory* inv = supplies->inventory;
	auto prevSlot = supplies->selectedHotbarSlot;

	for (int n = 0; n < 36; n++) {
		C_ItemStack* stack = inv->getItemStack(n);
		if (stack->item != nullptr) {
			if (stack->getItem()->isPickaxe()) {
				if (prevSlot != n) {
					supplies->selectedHotbarSlot = n;
				}
				return true;
			}
		}
	}
	return false;
}

void Regen::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	rot = false;
	bypassrot = false;
	firstbreak = false;
	continuemine = false;
	canattack = true;
	isregen = true;
	enabledticks = 0;
	blocksBroke = 0;

	animYawB = player->yawUnused1;
	animPitchB = player->pitch;

	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	slot = supplies->selectedHotbarSlot;
}

void Regen::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr || moduleMgr->getModule<Scaffold>()->isEnabled()) return;
	if (player->getAbsorption() == 10) return;
	//int health = player->getHealth();
	/*if (HealthLimitter && health > limithealth) {
		isregen = false;
		return;
	}else isregen = true;*/
	tick++;
	enabledticks++;
	C_PlayerInventoryProxy* supplies = g_Data.getLocalPlayer()->getSupplies();
	static AbsorptionAttribute attribute = AbsorptionAttribute();
	vec2_t angle = player->getPos()->CalcAngle(vec3_t(blockPos.x, blockPos.y, blockPos.z));
	auto timerUtil = new TimerUtil();
	static vector<vec3_ti> blocks;
	auto ka = moduleMgr->getModule<Killaura>();
	if (ka->atk) return;

	if (tick == delay) tick = 0;
	if (g_Data.canUseMoveKeys()) {
		switch (mode.getSelectedValue()) {
		case 0: // Hive
		{

			//Check Absorpription Health Check
			if (player->getAbsorption() >= 10) {
				isregen = false;
				continuemine = false;
				canattack = true;
				enabledticks = 0;
				rot = false; 
				breaknow = false;
				return;
			}
			else
			{
				isregen = true;
			}

			if (enabledticks > minedelay) {
				isregen = true;
			}
			else
			{
				isregen = false;
				canattack = false;
				animYawB = player->yawUnused1;
				animPitchB = player->pitch;
			}

			if (blocks.empty()) {
				for (int x = -range; x <= range; x++) {
					for (int z = -range; z <= range; z++) {
						for (int y = -range; y <= range; y++) {
							blocks.push_back(vec3_ti(x, y, z));
						}
					}
				}
				// https://www.mathsisfun.com/geometry/pythagoras-3d.html sort = a12 + a22 + √(x2 + y2 + z2) + an2 √(a12 + a22 + √(x2 + y2 + z2) + an2) / 4.f
				sort(blocks.begin(), blocks.end(), [](vec3_ti start, vec3_ti end) {
					return sqrtf((start.x * start.x) + (start.y * start.y) + (start.z * start.z)) < sqrtf((end.x * end.x) + (end.y * end.y) + (end.z * end.z));
					});
			}

			bool isDestroyed2 = false;

			for (const vec3_ti& offset : blocks) {
				blockPos = gm->player->getPos()->add(offset.toVector3());
				vec3_t blockPos2 = gm->player->getPos()->add(offset.toVector3());
				destroy = false;

				string name = gm->player->region->getBlock(blockPos)->toLegacy()->name.getText();
				vec3_ti AirCheckPos;
				bool isSafe = false;
				AirCheckPos = blockPos;
				//if (gm->player->region->getBlock(AirCheckPos.add(0, 1, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 1, 0))->toLegacy()->material->isReplaceable) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, -1, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(-1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, 1))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, -1))->toLegacy()->blockId == 0) isSafe = true;
				if (continuemine) {
					if (name == "lit_redstone_ore" || name == "redstone_ore" && tempblockPos == blockPos2 && isSafe) {
						destroy = true;
						//selectPickaxe();
						//gm->startDestroyBlock(blockPos, 1, isDestroyed);
						tempblockPos = blockPos2; // moved
					}
				}
				else {
					if (name == "lit_redstone_ore" || name == "redstone_ore" && isSafe) {
						destroy = true;
						//selectPickaxe();
						gm->startDestroyBlock(blockPos, 1, isDestroyed2);
						continuemine = true;
						tempblockPos = blockPos2;
						
					}
				}

				/*if (!destroy) {
					enabledticks = 0;
				}*/


				int aids = delay - 1;
				if (destroy/* && tick >= aids*/) {
					if (swing) player->swing();
					slot = supplies->selectedHotbarSlot;
					static bool restored = false;
					if (!selectPickaxe()) {
						if (!restored) restored = true;
						else {
							auto warning = g_Data.addNotification("Regen", "No Pickaxe Found"); warning->duration = 3;
							setEnabled(false);
						}
						return;
					}
					restored = false;
					if (isregen) rot = true;
					/*if (enabledticks > 2 && tempblockPos != OldBlockPos) {
						gm->stopDestroyBlock(OldBlockPos);
						enabledticks = 0;
						clientMessageF("BlockChanged");
					} */
					//if (enabledticks > 12 && isregen) clientMessageF("IsDestroyed");
					if ((enabledticks > minedelay && g_Data.canUseMoveKeys() && !moduleMgr->getModule<Killaura>()->atk && moduleMgr->getModule<Killaura>()->instarot) && isregen || (enabledticks > minedelay && g_Data.canUseMoveKeys() && !moduleMgr->getModule<Killaura>()->targetListEmpty && !moduleMgr->getModule<Killaura>()->instarot) && isregen) {
						player->bodyYaw = angle.y;
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
					}
					if (enabledticks > minedelay - 1)
					{
						breaknow = true;
					}
					else
					{
						breaknow = false;
					}
					if (firstbreak && enabledticks > minedelay) {
						gm->destroyBlock(&blockPos, 0); hasDestroyed = true; rot = false; enabledticks = 0; gm->stopDestroyBlock(blockPos); continuemine = false;
					}
					if (firstbreak) {
						firstbreak = false;
					}
					else firstbreak = true;
					supplies->selectedHotbarSlot = slot;
					if (!swing) player->swing();
					return;
				}

				if (hasDestroyed) { hasDestroyed = false; blocksBroke += 1; }
				if (blocksBroke > 4) blocksBroke = 0;

				//OldBlockPos = gm->player->getPos()->add(offset.toVector3());
			}

			if (!destroy) {
				//clientMessageF("Oof redstone not found");
				isregen = false;
				continuemine = false;
				canattack = true;
				enabledticks = 0;
				rot = false;
				breaknow = false;
				//clientMessageF("Lost Redstone");
			}
			else
			{
				canattack = true;
			}
		}
		break;
		case 1: // HiveTest
		{
			smooth = false;
			//Check Absorpription Health Check
			if (gm->player->getMutableAttribute(&attribute)->currentValue >= 10) {
				isregen = false;
				continuemine = false;
				canattack = true;
				enabledticks = 0;
				rot = false;
				breaknow = false;
				return;
			}
			else
			{
				isregen = true;
			}

			if (enabledticks > minedelay) {
				isregen = true;
			}
			else
			{
				isregen = false;
				canattack = false;
				animYawB = player->yawUnused1;
				animPitchB = player->pitch;
			}

			if (blocks.empty()) {
				for (int x = -range; x <= range; x++) {
					for (int z = -range; z <= range; z++) {
						for (int y = -range; y <= range; y++) {
							blocks.push_back(vec3_ti(x, y, z));
						}
					}
				}
				// https://www.mathsisfun.com/geometry/pythagoras-3d.html sort = a12 + a22 + √(x2 + y2 + z2) + an2 √(a12 + a22 + √(x2 + y2 + z2) + an2) / 4.f
				sort(blocks.begin(), blocks.end(), [](vec3_ti start, vec3_ti end) {
					return sqrtf((start.x * start.x) + (start.y * start.y) + (start.z * start.z)) < sqrtf((end.x * end.x) + (end.y * end.y) + (end.z * end.z));
					});
			}

			bool isDestroyed2 = false;

			for (const vec3_ti& offset : blocks) {
				blockPos = gm->player->getPos()->add(offset.toVector3());
				vec3_t blockPos2 = gm->player->getPos()->add(offset.toVector3());
				destroy = false;

				string name = gm->player->region->getBlock(blockPos)->toLegacy()->name.getText();
				vec3_ti AirCheckPos;
				bool isSafe = false;
				AirCheckPos = blockPos;
				/*if (gm->player->region->getBlock(AirCheckPos.add(0, 1, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, -1, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(-1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, 1))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, -1))->toLegacy()->blockId == 0) isSafe = true;*/
				if (gm->player->region->getBlock(AirCheckPos.add(0, 1, 0))->toLegacy()->material->isReplaceable) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, -1, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(-1, 0, 0))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, 1))->toLegacy()->blockId == 0) isSafe = true;
				if (gm->player->region->getBlock(AirCheckPos.add(0, 0, -1))->toLegacy()->blockId == 0) isSafe = true;
				if (continuemine) {
					if (name == "lit_redstone_ore" || name == "redstone_ore" && tempblockPos == blockPos2 && isSafe) {
						destroy = true;
						//selectPickaxe();
						//gm->startDestroyBlock(blockPos, 1, isDestroyed);
						tempblockPos = blockPos2; // moved
					}
				}
				else {
					if (name == "lit_redstone_ore" || name == "redstone_ore" && isSafe) {
						destroy = true;
						//selectPickaxe();
						gm->startDestroyBlock(blockPos, 1, isDestroyed2);
						continuemine = true;
						tempblockPos = blockPos2;
					}
				}

				/*if (!destroy) {
					enabledticks = 0;
				}*/


				int aids = delay - 1;
				if (destroy/* && tick >= aids*/) {
					if (swing) player->swing();
					slot = supplies->selectedHotbarSlot;
					static bool restored = false;
					if (!selectPickaxe()) {
						if (!restored) restored = true;
						else {
							auto warning = g_Data.addNotification("Regen", "No Pickaxe Found"); warning->duration = 3;
							setEnabled(false);
						}
						return;
					}
					restored = false;
					if (isregen) rot = true;
					/*if (enabledticks > 2 && tempblockPos != OldBlockPos) {
						gm->stopDestroyBlock(OldBlockPos);
						enabledticks = 0;
						clientMessageF("BlockChanged");
					} */
					//if (enabledticks > 12 && isregen) clientMessageF("IsDestroyed");
					if ((enabledticks > minedelay && g_Data.canUseMoveKeys() && !moduleMgr->getModule<Killaura>()->atk && moduleMgr->getModule<Killaura>()->instarot) && isregen || (enabledticks > minedelay && g_Data.canUseMoveKeys() && !moduleMgr->getModule<Killaura>()->targetListEmpty && !moduleMgr->getModule<Killaura>()->instarot) && isregen) {
						player->bodyYaw = angle.y;
						player->yawUnused1 = angle.y;
						player->pitch = angle.x;
						player->setRot(angle);
						MoveUtil::fullStop(true);
					}
					if (enabledticks > minedelay - 1)
					{
						breaknow = true;
					}
					else
					{
						breaknow = false;
					}
					if (firstbreak && enabledticks > minedelay + 3) {
						gm->destroyBlock(&blockPos, 0); hasDestroyed = true; rot = false; enabledticks = 0; gm->stopDestroyBlock(blockPos); continuemine = false;
					}
					if (firstbreak) {
						firstbreak = false;
					}
					else firstbreak = true;
					supplies->selectedHotbarSlot = slot;
					if (!swing) player->swing();
					return;
				}

				if (hasDestroyed) { hasDestroyed = false; blocksBroke += 1; }
				if (blocksBroke > 4) blocksBroke = 0;

				//OldBlockPos = gm->player->getPos()->add(offset.toVector3());
			}

			if (!destroy) {
				//clientMessageF("Oof redstone not found");
				isregen = false;
				continuemine = false;
				canattack = true;
				enabledticks = 0;
				rot = false;
				breaknow = false;
				//clientMessageF("Lost Redstone");
			}
			else
			{
				canattack = true;
			}
		}
		break;
		}
	}
}

void Regen::onPlayerTick(C_Player* plr) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	if (player->getAbsorption() == 10) return;
	vec2_t angle = player->getPos()->CalcAngle(vec3_t(blockPos.x, blockPos.y, blockPos.z));
	if (animYaw > angle.y)
		animYaw -= ((animYaw - angle.y) / 10);
	else if (animYaw < angle.y)
		animYaw += ((angle.y - animYaw) / 10);
	if (destroy) {
		//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&at);
	}
}
void Regen::onPreRender(C_MinecraftUIRenderContext* renderCtx) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	if(visual && destroy){
	if (blockPos != vec3_ti(0, 0, 0)) {
	DrawUtils::setColor(.75f, .25f, .5f, 1.f);
	DrawUtils::drawBox(blockPos.toVector3().add(0.f, 0.f, 0.f),
	blockPos.add(1).toVector3().add(0.f, 0.f, 0.f), .3f);
       }
    }
}
void Regen::onSendPacket(C_Packet* packet) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (packet->isInstanceOf<PlayerAuthInputPacket>() && breaknow) {
		auto* movePacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
		vec2_t angle = player->getPos()->CalcAngle(vec3_t(blockPos.x, blockPos.y, blockPos.z));
		if (animYaw > angle.y)
			animYaw -= ((animYaw - angle.y) / 10);
		else if (animYaw < angle.y)
			animYaw += ((angle.y - animYaw) / 10);
		if (destroy) {
			if (g_Data.canUseMoveKeys()) {
				movePacket->pos.y = angle.y;
				movePacket->pos.x = angle.x;
			}
		}
	}
}

void Regen::onDisable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	blocksBroke = 0;
	destroy = false;
	tick = 0;
	isregen = false;
	rot = false;
	bypassrot = false;
	canattack = true;
}
