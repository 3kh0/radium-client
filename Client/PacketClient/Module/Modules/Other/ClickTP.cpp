#include "ClickTP.h"
#include "../pch.h"

ClickTP::ClickTP() : IModule(0, Category::OTHER, "Teleports to where you are looking") {
	registerBoolSetting("ServerMode", &servermode, servermode);
	registerBoolSetting("Hand", &hand, hand);
}

const char* ClickTP::getModuleName() {
	return "ClickTP";
}

void ClickTP::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (hand && player->getSupplies()->inventory->getItemStack(player->getSupplies()->selectedHotbarSlot)->item != nullptr)
		return;

	if (g_Data.canUseMoveKeys() && g_Data.isRightClickDown() && !hasClicked) {
		hasClicked = true;

		vec3_ti block = g_Data.getLocalPlayer()->pointingStruct->block;
		if (block == vec3_ti(0, 0, 0)) return;

		vec3_t pos = block.toFloatVector();
		pos.x += 0.5f; pos.z += 0.5f; position = pos;

		position.y += (player->getPos()->y - player->getAABB()->lower.y) + 1;


		vec3_t pos2 = *player->getPos();
		vec3_t movepos = *player->getPos();
		int x = movepos.x;
		int y = movepos.y;
		int z = movepos.z;
		int ex = position.x;
		int ey = position.y;
		int ez = position.z;
		int gx = pos2.x;
		int gy = pos2.y;
		int gz = pos2.z;
		bool isplus = false;
		int counter = 0;
		//TP
		if (servermode) {
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
				clientMessageF("x moved");
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
				C_MovePlayerPacket packet(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); packet.Position.y = position.y;
				g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&packet);
				PlayerAuthInputPacket p;
				p.pos.y = y;
				//g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p);
				clientMessageF("y moved");
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
				clientMessageF("z moved");
			}

			clientMessageF("teleported");
		}
		player->setPos(position);
		C_MovePlayerPacket oldpospacket(g_Data.getLocalPlayer(), *g_Data.getLocalPlayer()->getPos()); oldpospacket.Position = position;
		g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&oldpospacket);


	}
	if (!g_Data.isRightClickDown()) hasClicked = false;
}

void ClickTP::onPostRender(C_MinecraftUIRenderContext* renderCtx) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (g_Data.canUseMoveKeys() && hasClicked) {
		PointingStruct* pointing = g_Data.getLocalPlayer()->pointingStruct;

		vec3_t block = pointing->block.toVector3();
		block = block.floor();
		vec3_t outline = pointing->block.toVector3();
		outline = outline.floor();
		outline.x += 1.f;
		outline.y += 1.f;
		outline.z += 1.f;

		DrawUtils::setColor(0, 1, 0, 1);
		DrawUtils::drawBox(block, outline, 0.6, 1);
	}
}