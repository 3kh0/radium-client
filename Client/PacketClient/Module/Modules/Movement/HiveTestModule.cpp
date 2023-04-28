#include "HiveTestModule.h"

HiveTestModule::HiveTestModule() : IModule(0, Category::MOVEMENT, "TestModuleForHive") {
	registerBoolSetting("Clip Up", &clip, clip);
	registerFloatSetting("Clip Height", &clipHeight, clipHeight, 0.f, 5.f);
	registerFloatSetting("Down", &fall, fall, 1.f, 2.f);
	registerIntSetting("HiveDelay", &HiveDelay, HiveDelay, 0, 10);
	registerFloatSetting("HiveVelocity", &HiveVelocity, HiveVelocity, -1.00f, -0.00f);
}

const char* HiveTestModule::getModuleName() {
	return ("HiveTestModule");
}

float hiveSpeedArrayB[12] = {
0.40000,
0.38000,
0.36000,
0.34000,
0.32000,
0.30000,
0.29000,
0.28000,
0.26000,
0.24000,
0.22000,
0.20347
};

int flySpeedIndexB = 0;

int stopYThingyB = 0;

void HiveTestModule::onEnable() {
	srand(time(NULL));
	counter = 0;
	counter69 = 0;
	flySpeedIndexB = 0;
	stopYThingyB = 0;
	falldist = 0;
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	if (player != nullptr) {
		/*if (player->onGround == true) {
			player->jumpFromGround();
			downf = -0.05f;
		}
		else
		{
			counter69 = 6;
			if (player->velocity.y > 0) {
				downf = -0.05f;
			}
			else
			{
				downf = player->velocity.y;
			}
		}*/
		if (player->onGround == true) {
			vec3_t myPos = *player->getPos();
			myPos.y += clipHeight;
			player->setPos(myPos);
			counter69 = 6;
		}
	}
}

void HiveTestModule::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	player->setSprinting(true);
}

void HiveTestModule::onMove(C_MoveInputHandler* input) {
	counter69++;
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	if (player == nullptr) return;


	bool pressed = MoveUtil::isMoving();
	float calcYaw = (player->yaw + 90) * (PI / 180);
	float c = cos(calcYaw);
	float s = sin(calcYaw);

	vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
	moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };
	vec3_t moveVec;

	//HiveSafe by DarkNBTHax
		hiveVelocity++;
		enabledTick++;
		if (counter69 > 5) {
			*g_Data.getClientInstance()->minecraft->timer = 6.f;
			float safeSpeedArray = hiveSpeedArrayB[hiveSpeedIndex++ % 12];

			if (pressed) {
				moveVec.x = moveVec2D.x * safeSpeedArray; moveVec.z = moveVec2D.y * safeSpeedArray;

				if (hiveVelocity >= HiveDelay) { hiveVelocity = 0; moveVec.y = player->velocity.y; falldist -= player->velocity.y; }
				if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
				player->fallDistance = falldist;
			}
			else MoveUtil::stop(false);
		}
	}

void HiveTestModule::onSendPacket(C_Packet* packet) {
	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (packet->isInstanceOf<C_MovePlayerPacket>()) {
		auto* movePacket = reinterpret_cast<C_MovePlayerPacket*>(packet);
		PlayerAuthInputPacket* authInputPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
		movePacket->onGround = true;
	}
}

void HiveTestModule::onDisable() {
	counter69 = 0;
	counter = 0;
	if (g_Data.getLocalPlayer() != nullptr) {
		C_LocalPlayer* player = g_Data.getLocalPlayer();
/*		player->velocity.x = 0.f;
		player->velocity.y = 0.f;
		player->velocity.z = 0.f;*/
		player->velocity.x = 0.f;
		player->velocity.z = 0.f;
		*g_Data.getClientInstance()->minecraft->timer = 20.f;
	}
}