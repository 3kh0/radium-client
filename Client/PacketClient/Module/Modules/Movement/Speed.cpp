#include "Speed.h"
#include "../pch.h"

bool testOnGround = false;
int packetsSent = 0;
int enabledTicks = 0;
int flareonticks = 0;
int strafeTime = 0;
int hivegroundticks = 0;
float effectiveValue = 0.f;
float random2 = 0.f;
float random3 = 0.f;
float fricspeed = 0.f;
using namespace std;
Speed::Speed() : IModule(0, Category::MOVEMENT, "Increases your speed") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Vanilla", 0);
	mode.addEntry("Hive", 1);
	mode.addEntry("HiveSafe", 2);
	mode.addEntry("HiveGround", 3);
	mode.addEntry("Cubecraft", 4);
	mode.addEntry("Friction", 5);
	mode.addEntry("Boost", 6);
	mode.addEntry("Test", 7);
	mode.addEntry("NoGround", 8);
	//mode.addEntry("Packet", 9);
	mode.addEntry("TPBoost", 9);
	mode.addEntry("Halcyon", 10);
	mode.addEntry("DamageSafe", 11);
#ifdef _DEBUG
	mode.addEntry("DmgFlySpeed", 12); //only beta
#endif // _DEBUG

	registerFloatSetting("Height", &height, height, 0.000001f, 0.40f);
	registerFloatSetting("Speed", &speed, speed, 0.2f, 2.f);
	registerIntSetting("Timer", &timer, timer, 10, 40);
	registerFloatSetting("Random", &random2, random2, 0.f, 0.5f);
	registerFloatSetting("Duration", &duration, duration, 0.5f, 1.05f);
	registerBoolSetting("NoSlabs", &noslabs, noslabs);
	registerIntSetting("BypassTime", &bypasstime, bypasstime, 1, 20);
	registerIntSetting("DamageSpeed", &damagespeed, damagespeed, 0, 10); //only beta
	registerIntSetting("DamageTimer", &dmgtimer, dmgtimer, 1, 40);
	///registerBoolSetting("FullStop", &fullstop, fullstop); // Prevents flags cause instantly stops
	//registerBoolSetting("DesnycBoost", &dboost, dboost);
	//registerBoolSetting("Rotate", &rotate, rotate);
}

const char* Speed::getRawModuleName() {
	return "Speed";
}

const char* Speed::getModuleName() {
	name = string("Speed ") + string(GRAY) + mode.GetEntry(mode.getSelectedValue()).GetName();
	return name.c_str();
}

void Speed::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	enabledTicks = 0;
	flareonticks = 0;
	strafeTime = 0;
	clientmessage = false;
	hivegroundticks = 0;
	needblink = false;
	damageMotion = 0;
	strafeTime = 50;

	oldx = player->currentPos.x;
	oldz = player->currentPos.z;
	animYaw = player->yaw;
}

void Speed::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (enabledTicks > 10) enabledTicks++;
	else enabledTicks = 0;

	if (player->onGround) { groundTicks++; offGroundTicks = 0; }
	else { offGroundTicks++; groundTicks = 0; }

	if (noslabs) player->stepHeight = 0.3f;

	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	//g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);

	// HiveLow
	switch (mode.getSelectedValue()) {
	case 3: // Cubecraft
		ticks++;
		break;
	}
	if (dboost) {
		if (player->onGround) {
			moduleMgr->getModule<Blink>()->setEnabled(false);
			//auto notification = g_Data.addNotification("Speed:", "IKA HA BAKA"); notification->duration = 3;
		}

		if (GameData::isKeyDown(*input->spaceBarKey))g_Data.getClientInstance()->minecraft->setTimerSpeed(boosttimer);
		if (GameData::isKeyDown(*input->spaceBarKey)) {
			if (player->onGround) {
				moduleMgr->getModule<Blink>()->setEnabled(false);
				//auto notification = g_Data.addNotification("Speed:", "INSANE BYPASS"); notification->duration = 3;
			}
			else
			{
				moduleMgr->getModule<Blink>()->setEnabled(true);
			}
		}

		if (!GameData::isKeyDown(*input->spaceBarKey)) {
			moduleMgr->getModule<Blink>()->setEnabled(false);
		}
	}
}

void Speed::onMove(C_MoveInputHandler* input) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);
	bool pressed = MoveUtil::keyPressed();
	if (!pressed && fullstop) MoveUtil::stop(false);
	if (mode.getSelectedValue() != 10) player->setSprinting(true);

	float yaw = player->yaw;

	if (input->forward && input->backward) {
	}
	else if (input->forward && input->right && !input->left) {
		yaw += 45.f;
	}
	else if (input->forward && input->left && !input->right) {
		yaw -= 45.f;
	}
	else if (input->backward && input->right && !input->left) {
		yaw += 135.f;
	}
	else if (input->backward && input->left && !input->right) {
		yaw -= 135.f;
	}
	else if (input->forward) {
	}
	else if (input->backward) {
		yaw += 180.f;
	}
	else if (input->right && !input->left) {
		yaw += 90.f;
	}
	else if (input->left && !input->right) {
		yaw -= 90.f;
	}

	if (animYaw > yaw) animYaw -= ((animYaw - yaw) / 6);
	else if (animYaw < yaw) animYaw += ((yaw - animYaw) / 6);

	// Vanilla
	if (mode.getSelectedValue() == 0) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) { if (player->onGround && pressed) player->jumpFromGround(); useVelocity = false; }
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		if (player->onGround && useVelocity && pressed && !input->isJumping) player->velocity.y = height;
		random3 = 0 - random2;
		fricspeed = randomFloat(random2, random3);
		MoveUtil::setSpeed(speed + fricspeed);
	}

	// Hive
	if (mode.getSelectedValue() == 1) {
		speedFriction *= 0.9400610828399658f;
		if (pressed) {
			if (player->onGround) {
				player->jumpFromGround();
				//speedFriction = randomFloat(0.5285087823867798f, 0.49729517102241516f);
				speedFriction = 0.5185087823867798f;
			}
			else MoveUtil::setSpeed(speedFriction);
		}
	}

	// HiveSafe
	if (mode.getSelectedValue() == 2) {
		speedFriction *= 0.9500610828399658f;
		if (pressed) {
			if (player->onGround) {
				player->jumpFromGround();
				speedFriction = randomFloat(0.5085087823867798f, 0.47729517102241516f);
			}
			else MoveUtil::setSpeed(speedFriction);
		}
	}

	// HiveGround
	if (mode.getSelectedValue() == 3) {
		// eat my absctrionalie
		//if (player->onGround && pressed) { player->jumpFromGround(); }

		//speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
				if (hivegroundticks < 1) {
					player->jumpFromGround();
					if (0 < player->velocity.y) player->velocity.y = 0.f;
					hivegroundticks = 3;
				}
				hivegroundticks--;
			}
		}
	}

	// Cubecraft
	if (mode.getSelectedValue() == 4) {
		float calcYaw = (player->yaw + 90) * (PI / 180);
		float c = cos(calcYaw);
		float s = sin(calcYaw);

		vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
		moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };
		vec3_t moveVec;

		speedFriction *= 0.97;
		if (ticks % 3 == 0) {
			moveVec.x = moveVec2D.x * speedFriction;
			moveVec.z = moveVec2D.y * speedFriction;
			moveVec.y = player->velocity.y;
		}
		else {
			g_Data.getClientInstance()->minecraft->setTimerSpeed(17.f);
			moveVec.x = moveVec2D.x * speedFriction / 3;
			moveVec.z = moveVec2D.y * speedFriction / 3;
			moveVec.y = player->velocity.y;
		}

		if (pressed) {
			player->lerpMotion(moveVec);
			if (player->onGround) {
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
				player->jumpFromGround();
			}
		}
	}

	// Friction
	if (mode.getSelectedValue() == 5) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) {
			if (player->onGround && pressed) { player->jumpFromGround(); useVelocity = false; }
		}
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				if (useVelocity && !input->isJumping) player->velocity.y = height;
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
			}
			else MoveUtil::setSpeed(speedFriction);
		}
	}

	// Fast / timerboost
	if (mode.getSelectedValue() == 9) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) {
			if (player->onGround && pressed) { player->jumpFromGround(); useVelocity = false; }
		}
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				if (useVelocity && !input->isJumping) player->velocity.y = height;
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
				g_Data.getClientInstance()->minecraft->setTimerSpeed(100.f);
			}
			else {
				MoveUtil::setSpeed(speedFriction);
				g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);
			}
		}
	}

	// Halcyon
	if (mode.getSelectedValue() == 10) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) {
			if (player->onGround && pressed) { 
				//player->setRot(vec2_t(yaw, player->pitch));
				player->jumpFromGround(); 
				useVelocity = false; 
			}
		}
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { 
			//player->setRot(vec2_t(yaw, player->pitch));
			player->jumpFromGround(); 
			player->velocity.y += height; 
			useVelocity = false; 
		}

		if (speedFriction > 0.25) speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				if (useVelocity && !input->isJumping) player->velocity.y = height;
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
			}
			else MoveUtil::setSpeed(speedFriction);
		}
	}

	/* HiveACDisabler
	if (clientmessage) {
		clientMessageF("AntiCheat got disabled");
		clientmessage = false;
	}

	if (mode.getSelectedValue() == 11) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) {
			if (player->onGround && pressed) { player->jumpFromGround(); useVelocity = false; }
		}
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				if (useVelocity && !input->isJumping) player->velocity.y = height;
				speedFriction = randomFloat(speedMin, speedMax);
				g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
				g_Data.getClientInstance()->minecraft->setTimerSpeed(100.f);
			}
			else {
				MoveUtil::setSpeed(speedFriction);
				g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
			}
		}
	}*/

	// Flareon
	if (mode.getSelectedValue() == 8) {
		static bool useVelocity = false;
		flareonticks++;
		// eat my absctrionalie
		if (height >= 0.385) { if (player->onGround && pressed) { player->jumpFromGround(); useVelocity = false; } }
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		if (flareonticks >= 10 && speedFriction >= 0.4) {
			speedFriction = 0.2;
		}
		else speedFriction *= duration;
		if (pressed) {
			if (player->onGround) {
				if (useVelocity && !input->isJumping) player->velocity.y = height;
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
				MoveUtil::stop(false);
				flareonticks = 0;
			}
			else MoveUtil::setSpeed(speedFriction);
		}
	}

	// Boost
	if (mode.getSelectedValue() == 6) {
		static bool useVelocity = false;
		// eat my absctrionalie
		if (height >= 0.385) { if (player->onGround && pressed) player->jumpFromGround(); useVelocity = false; }
		else useVelocity = true;
		if (height <= 0.04 && !input->isJumping) { player->velocity.y += height; useVelocity = false; }

		if (pressed && player->onGround) {
			if (useVelocity && !input->isJumping) player->velocity.y = height;
			random3 = 0 - random2;
			fricspeed = randomFloat(random2, random3);
			MoveUtil::setSpeed(speed + fricspeed);
		}
	}

	// Test
	if (mode.getSelectedValue() == 7) {
		float calcYaw = (player->yaw - 90) * (PI / 180);
		float calcPitch = (player->pitch) * (PI / 180);
		vec3_t rotVec;
		rotVec.x = cos(calcYaw) * randomFloat(.5f, 1.f);
		rotVec.y = sin(calcPitch) * randomFloat(.5f, 1.f);
		rotVec.z = sin(calcYaw) * randomFloat(.5f, 1.f);

		if (pressed) {
			if (MoveUtil::isMoving()) {
				C_MovePlayerPacket p2 = C_MovePlayerPacket(g_Data.getLocalPlayer(), player->getPos()->add(vec3_t(rotVec.x / randomFloat(1.1f, 2.2f), 0.f, rotVec.z / randomFloat(1.1f, 2.2f))));
				g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&p2);
				packetsSent++;
			}
			if (player->onGround) {
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				speedFriction = speed + fricspeed;
			}
			else {
				speedFriction = randomFloat(.33f, .45f);
			}
			MoveUtil::setSpeed(speedFriction);
		}
	}

	//DamageBoost
	if (mode.getSelectedValue() == 11) {
		if (pressed) {
			if (player->onGround) {
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				MoveUtil::setSpeed(speed + fricspeed);
				player->jumpFromGround();
			}

			if (damageMotion != 0 && damageMotion >= 0.15f) {
				setSpeed(damageMotion + speed * (damagespeed / 10));
				g_Data.getClientInstance()->minecraft->setTimerSpeed(dmgtimer);
				damageMotion = 0;
				strafeTime = 0;
			}

			if (MoveUtil::isMoving()) {
				if (strafeTime <= bypasstime) {
					strafeTime++;
					setSpeed(player->velocity.magnitudexz());
				}
				else
				{
					g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);
				}
			}
		}
	}
	//DamageFly       only beta
	if (mode.getSelectedValue() == 12) {
		if (pressed) {
			if (player->onGround) {
				random3 = 0 - random2;
				fricspeed = randomFloat(random2, random3);
				MoveUtil::setSpeed(speed + fricspeed);
				player->jumpFromGround();
			}

			if (damageMotion != 0 && damageMotion >= 0.15f) {
				setSpeed((damageMotion + speed) * damagespeed);
				damageMotion = 0;
				strafeTime = 0;
			}

			if (MoveUtil::isMoving()) {
				if (strafeTime < bypasstime) {
					strafeTime++;
					g_Data.getClientInstance()->minecraft->setTimerSpeed(dmgtimer);
					MoveUtil::setSpeed((damageMotion + speed) * damagespeed);

					float calcYaw = (player->yaw + 90) * (PI / 180);
					float c = cos(calcYaw);
					float s = sin(calcYaw);

					vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
					moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };
					vec3_t moveVec;
					player->velocity.y = 0;
				}
				else
				{
					if (strafeTime == bypasstime + 1)
					{
						MoveUtil::setSpeed(0);
						MoveUtil::fullStop(true);
						player->velocity.x = 0;
						player->velocity.y = 0;
						player->velocity.z = 0;
						g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);
					}
				}

			}
		}
	}
}

void Speed::setSpeed(float speed) {
	C_MoveInputHandler* input = g_Data.getClientInstance()->getMoveTurnInput();
	auto player = g_Data.getLocalPlayer();
	float calcYaw = (player->yaw + 90) * (PI / 180);
	float c = cos(calcYaw);
	float s = sin(calcYaw);

	vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
	moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };
	vec3_t moveVec;

	moveVec.x = moveVec2D.x * speed;
	moveVec.y = player->velocity.y;
	moveVec.z = moveVec2D.y * speed;
	player->lerpMotion(moveVec);
}

void Speed::onSendPacket(C_Packet* packet) {
	C_GameSettingsInput* input = g_Data.getClientInstance()->getGameSettingsInput();
	auto scaffold = moduleMgr->getModule<Scaffold>();
	auto player = g_Data.getLocalPlayer();
	auto* LatencyPacket = reinterpret_cast<NetworkLatencyPacket*>(packet);
	auto* movePacket = reinterpret_cast<C_MovePlayerPacket*>(packet);
	auto* authinputPacket = reinterpret_cast<PlayerAuthInputPacket*>(packet);
	NetworkLatencyPacket* netStack = (NetworkLatencyPacket*)packet;
	if (player == nullptr || input == nullptr) return;

	if (packet->isInstanceOf<C_MovePlayerPacket>()) {
		//packet
		if (moduleMgr->getModule<Regen>()->breaknow) return;
		if (!moduleMgr->getModule<Killaura>()->targetListEmpty) return;
		if (!rotate) return;
		movePacket->yaw = animYaw;
		movePacket->headYaw = animYaw;
	}
}

void Speed::onDisable() {
	g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	if (fullstop) MoveUtil::stop(false);

	preventKick = false;
	packetsSent = 0;
	needblink = false;
	player->stepHeight = 0.5625f;
}
