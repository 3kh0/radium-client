#include "TPAura.h"
#include "../pch.h"

using namespace std;
TPAura::TPAura() : IModule(0, Category::COMBAT, "Attacks entities from far ranges") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Multi", 0);
	mode.addEntry("Switch", 1);
	registerIntSetting("Delay", &delay, delay, 1, 50);
	registerFloatSetting("Range", &range, range, 5.f, 250.f);
}

const char* TPAura::getModuleName() {
	return ("TPAura");
}

static vector<C_Entity*> targetList;
void findEntTPA(C_Entity* currentEntity, bool isRegularEntity) {
	static auto tpaura = moduleMgr->getModule<TPAura>();

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

	if (currentEntity->getEntityTypeId() == 80 || currentEntity->getEntityTypeId() == 69)  // XP and Arrows
		return;

	if (!TargetUtil::isValidTarget(currentEntity))
		return;

	if (currentEntity->getEntityTypeId() == 51 || currentEntity->getEntityTypeId() == 1677999)  // Villagers and NPCS
		return;

	float dist = (*currentEntity->getPos()).dist(*g_Data.getLocalPlayer()->getPos());
	if (dist < tpaura->range) targetList.push_back(currentEntity);
}

void TPAura::onEnable() {
	teleported = false; ticks = 0;
	targetList.clear();
}

struct CompareTargetEnArray {
	bool operator()(C_Entity* lhs, C_Entity* rhs) {
		C_LocalPlayer* localPlayer = g_Data.getLocalPlayer();
		return (*lhs->getPos()).dist(*localPlayer->getPos()) < (*rhs->getPos()).dist(*localPlayer->getPos());
	}
};

void TPAura::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	ticks++;

	int APS = 20;
	targetList.clear();
	g_Data.forEachEntity(findEntTPA);

	vec3_t entPos = *targetList[0]->getPos();
	vec3_t pos = *player->getPos();
	vec3_t movepos = *player->getPos();
	int x = movepos.x;
	int y = movepos.y;
	int z = movepos.z;
	int ex = entPos.x;
	int ey = entPos.y;
	int ez = entPos.z;
	int gx = pos.x;
	int gy = pos.y;
	int gz = pos.z;
	bool isplus = false;
	int counter = 0;
	//TP
	if (!teleported) {
		if (ticks < delay && ticks > 1) {
			//X
			counter = ex - x;
			if (0 < ex - x) {
				isplus = true;
			}
			else
			{
				isplus = false;
				counter = 0 - counter;
			}
			for (int i = 0; i < counter; i++) {
				if (isplus) {
					x += 1;
				}
				else {
					x -= 1;
				}
				C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.x = x;
				g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
				PlayerAuthInputPacket p;
				p.pos.x = x;
				//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
			}

			//Y
			counter = ey - y;
			if (0 < ey - y) {
				isplus = true;
			}
			else
			{
				isplus = false;
				counter = 0 - counter;
			}
			for (int i = 0; i < counter; i++) {
				if (isplus) {
					y += 1;
				}
				else {
					y -= 1;
				}
				C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.y = entPos.y + 2;
				g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
				PlayerAuthInputPacket p;
				p.pos.y = y;
				//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
			}

			//Z
			counter = ez - z;
			if (0 < ez - z) {
				isplus = true;
			}
			else
			{
				isplus = false;
				counter = 0 - counter;
			}
			for (int i = 0; i < counter; i++) {
				if (isplus) {
					z += 1;
				}
				else {
					z -= 1;
				}
				C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.z = z;
				g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
				PlayerAuthInputPacket p;
				p.pos.z = z;
				//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
			}
			clientMessageF("teleported");
			teleported = true;
			C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position = entPos;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
		}
	}
	else
	{
		//Continue tp
		C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position = entPos;
		//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
		PlayerAuthInputPacket p;
		p.pos = entPos;
		//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
		clientMessageF("Continue teleported");
	}


	//Attack
	if (g_Data.canUseMoveKeys() && !targetList.empty() /* && teleported*/) {
		if (TimerUtil::hasTimedElapsed((1000 / APS), true)) {
			if (!moduleMgr->getModule<NoSwing>()->isEnabled())
				player->swing();
				gm->attack(targetList[0]);
				clientMessageF("attacked");

			/*for (auto& i : targetList) {
				// crashes
				if (mode.getSelectedValue() == 0) gm->attack(i);
				else gm->attack(targetList[0]);
				clientMessageF("attacked");
			}*/
		}
	}

	//Setback
	if (ticks >= delay) {
		//X
		counter = gx - x;
		if (0 < gx - x) {
			isplus = true;
		}
		else
		{
			isplus = false;
			counter = 0 - counter;
		}
		for (int i = 0; i < counter; i++) {
			if (isplus) {
				x += 1;
			}
			else {
				x -= 1;
			}
			C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.x = x;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
			PlayerAuthInputPacket p;
			p.pos.x = x;
			//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
		}

		//Y
		counter = gy - y;
		if (0 < gy - y) {
			isplus = true;
		}
		else
		{
			isplus = false;
			counter = 0 - counter;
		}
		for (int i = 0; i < counter; i++) {
			if (isplus) {
				y += 1;
			}
			else {
				y -= 1;
			}
			C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.y = pos.y;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
			PlayerAuthInputPacket p;
			p.pos.y = y;
			//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
		}

		//Z
		counter = gz - z;
		if (0 < gz - z) {
			isplus = true;
		}
		else
		{
			isplus = false;
			counter = 0 - counter;
		}
		for (int i = 0; i < counter; i++) {
			if (isplus) {
				z += 1;
			}
			else {
				z -= 1;
			}
			C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.z = z;
			g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
			PlayerAuthInputPacket p;
			p.pos.z = z;
			//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
		}


		teleported = false;
		ticks = 0;
		clientMessageF("set back");
		C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position = pos;
		g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
	}
}

void TPAura::onPostRender(C_MinecraftUIRenderContext* renderCtx) {
}

void TPAura::onLevelRender() {
}

void TPAura::onSendPacket(C_Packet* packet) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (packet->isInstanceOf<C_MovePlayerPacket>()) {
		auto* movePacket = reinterpret_cast<C_MovePlayerPacket*>(packet);
		auto* AuthinputPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);

		static int ticksSinceTeleport = 0;
		/*if (g_Data.canUseMoveKeys() && !targetList.empty()) {
			vec3_t entPos = *targetList[0]->getPos();
			vec3_t pos = *player->getPos();

			if (ticks % delay == 0) {
				clientMessageF("teleported");
				movePacket->Position.x = entPos.x;
				movePacket->Position.y = entPos.y;
				movePacket->Position.z = entPos.z;
				AuthinputPacket->pos.x = entPos.x;
				AuthinputPacket->pos.y = entPos.y;
				AuthinputPacket->pos.z = entPos.z;
				teleported = true;
				ticksSinceTeleport++;
			}

			if (ticksSinceTeleport >= 5) {
				teleported = false;
				clientMessageF("set back");
				movePacket->Position.x = pos.x; movePacket->Position.y = pos.y; movePacket->Position.z = pos.z;
				AuthinputPacket->pos.x = pos.x; AuthinputPacket->pos.y = pos.y; AuthinputPacket->pos.z = pos.z;
				ticksSinceTeleport = 0;
			}
		}*/
	}
}

void TPAura::onDisable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position = *player->getPos();
	g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
	PlayerAuthInputPacket p;
	p.pos = *player->getPos();
	//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
}