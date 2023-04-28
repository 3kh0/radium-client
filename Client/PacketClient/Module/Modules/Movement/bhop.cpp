#include "bhop.h"
#include "../pch.h"

bool testOnGround2 = false;
float speedMax2 = 0.69f; // inf value
float speedMin2 = 0.59f; // inf value
int packetsSent2 = 0;
int enabledTicks2 = 0;
int flareonticks2 = 0;
bool falled = false;

using namespace std;
Bhop::Bhop() : IModule(0, Category::MOVEMENT, "Full Bhop") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Normal", 0);
	mode.addEntry("Vector", 1);
	//Vannila
	registerBoolSetting("Horizontal", &horizontal, horizontal);
	registerBoolSetting("FastFall", &fastfall, fastfall);
	// All Settings
	registerIntSetting("Timer", &timer2, timer2, 20, 35);

	// Friction
	registerFloatSetting("SpeedMax", &speedMax2, speedMax2, 0.f, 2.f);
	registerFloatSetting("SpeedMin", &speedMin2, speedMin2, 0.f, 2.f);
	registerFloatSetting("Duration", &duration2, duration2, 0.5f, 1.05f);
}

const char* Bhop::getRawModuleName() {
	return "Bhop";
}

const char* Bhop::getModuleName() {
	name = string("Bhop ") + string(GRAY) + mode.GetEntry(mode.getSelectedValue()).GetName();
	return name.c_str();
}

void Bhop::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	enabledTicks2 = 0;
	flareonticks2 = 0;
	falled = false;
}

void Bhop::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (enabledTicks2 > 10) enabledTicks2++;
	else enabledTicks2 = 0;

	if (player->onGround) { groundTicks++; offGroundTicks = 0; }
	else { offGroundTicks++; groundTicks = 0; }

	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	g_Data.getClientInstance()->minecraft->setTimerSpeed(timer2);
}

void Bhop::onMove(C_MoveInputHandler* input) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	bool pressed = MoveUtil::keyPressed();
	if (!pressed) MoveUtil::stop(false);
	player->setSprinting(true);

	// Speed
		if (player->onGround && pressed) { player->jumpFromGround(); }

		speedFriction2 *= duration2;
		if (pressed) {
			if (player->onGround) {
				speedFriction2 = randomFloat(speedMin2, speedMax2);
				if(horizontal) MoveUtil::stop(false);
				falled = false;
			}
			else {
				if (mode.getSelectedValue() == 0) {
					MoveUtil::setSpeed(speedFriction2);
				}
				else
				{
					float calcYaw = (player->yaw + 90) * (PI / 180);
					float c = cos(calcYaw);
					float s = sin(calcYaw);
					vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
					moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };
					vec3_t moveVec;

					moveVec.x = moveVec2D.x * speedFriction2; moveVec.z = moveVec2D.y * speedFriction2; moveVec.y = player->velocity.y;
					player->lerpMotion(moveVec);
				}
			}
		}
		if (fastfall && !falled && player->velocity.y < 0) {
			player->velocity.y = -0.15f;
			falled = true;
		}
}

void Bhop::onSendPacket(C_Packet* packet) {
	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	auto scaffold = moduleMgr->getModule<Scaffold>();
	auto player = g_Data.getLocalPlayer();
	auto* LatencyPacket = reinterpret_cast<NetworkLatencyPacket*>(packet);
	auto* movePacket = reinterpret_cast<C_MovePlayerPacket*>(packet);
	NetworkLatencyPacket* netStack = (NetworkLatencyPacket*)packet;
	if (player == nullptr || input == nullptr) return;
}

void Bhop::onDisable() {
	g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	MoveUtil::stop(false);

	preventKick2 = false;
	packetsSent2 = 0;
}