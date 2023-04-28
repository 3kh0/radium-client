#include "HiveFly.h"

HiveFly::HiveFly() : IModule(0, Category::MOVEMENT, "How the fuck does this bypass ?!?!?") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Custom", 0);
	mode.addEntry("Hive", 1);
	mode.addEntry("HiveSafe", 2);
	mode.addEntry("HiveTest", 3);
	mode.addEntry("GrassCutter", 4);
	registerEnumSetting("UpMode", &upmode, 0);
	upmode.addEntry("None", 0);
	upmode.addEntry("Jump", 1);
	upmode.addEntry("Clip", 2);
	upmode.addEntry("HiveClip", 3);
	registerFloatSetting("Clip Height", &clipHeight, clipHeight, 0.f, 5.f);
	registerBoolSetting("AutoSprint", &sprint, sprint);
	registerFloatSetting("BoostSpeed", &speed, speed, 0.2f, 3.f);
	registerFloatSetting("BoostValue", &value, value, -1.f, 1.f);
	registerIntSetting("BoostDelay", &delay, delay, 0, 15);
	registerIntSetting("Timer", &timer, timer, 1, 80);
	registerIntSetting("CanFlyTicks", &canflytick, canflytick, 5, 100);
}

const char* HiveFly::getModuleName() {
	return ("HiveFly");
}

float epicHiveFlySpeedArrayThingy[12] = {
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
0.20347 };

int flySpeedIndex = 0;

int stopYThingy = 0;

void HiveFly::onEnable() {
	srand(time(NULL));
	counter = 0;
	counter69 = 0;
	ticks = 0;
	flySpeedIndex = 0;
	friction = 0.4f;
	stopYThingy = 0;
	if (mode.getSelectedValue() == 4) clipHeight = 1.6f;
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	if (player != nullptr) {
		vec3_t myPos = *player->getPos();
		if (player->onGround) {
			switch (upmode.getSelectedValue()) {
			case 0: //None
				counter69 = 8;
				break;
			case 1: //Jump
				counter69++;
				if (counter69 <= 2) {
					player->jumpFromGround();
				}
				break;
			case 2: //Clip
				myPos.y += clipHeight;
				player->setPos(myPos);
				counter69 = 8;
				break;
			case 3: //HiveClip
				myPos.y += clipHeight;
				player->jumpFromGround();
				player->setPos(myPos);
				player->velocity.y = 0.f;
				counter69 = 8;
				break;
			}
		}
		else
		{
			counter69 = 8;
		}
		flypos = *player->getPos();
	}
}

void HiveFly::onTick(C_GameMode* gm) {
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	//if (mode.getSelectedValue() == 4) g_Data.getClientInstance()->minecraft->setTimerSpeed(24.f);

	if (canflytick == 100 && mode.getSelectedValue() == 0 && counter69 > 9) counter69 = 9;
}

void HiveFly::onMove(C_MoveInputHandler* input) {
	counter69++;
	ticks++;
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (sprint || mode.getSelectedValue() == 1 || mode.getSelectedValue() == 4 || mode.getSelectedValue() == 2) player->setSprinting(true);

	if (mode.getSelectedValue() == 0 && counter69 >= 5 && counter69 <= canflytick) g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);

	if (mode.getSelectedValue() == 1 && counter69 >= 5 && counter69 <= 20) g_Data.getClientInstance()->minecraft->setTimerSpeed(6.f);

	if (mode.getSelectedValue() == 2 && counter69 >= 5 && counter69 <= 20) g_Data.getClientInstance()->minecraft->setTimerSpeed(6.f);

	//if (mode.getSelectedValue() == 4) g_Data.getClientInstance()->minecraft->setTimerSpeed(24.f);

	if (canflytick == 100 && mode.getSelectedValue() == 0 && counter69 > 9) counter69 = 9;
	vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
	bool pressed = moveVec2d.magnitude() > 0.01f;

	float calcYaw = (player->yaw + 90) * (PI / 180);
	vec3_t moveVec;
	float c = cos(calcYaw);
	float s = sin(calcYaw);
	vec2_t moveVec2D = { input->forwardMovement, -input->sideMovement };
	moveVec2D = { moveVec2D.x * c - moveVec2D.y * s, moveVec2D.x * s + moveVec2D.y * c };

	//HiveTest by DarkNBTHax / Custom
	if (mode.getSelectedValue() == 0) {
		if (counter69 <= canflytick) {
			if (pressed) {
				//player->jumpFromGround();
				moveVec.x = moveVec2D.x * speed; moveVec.z = moveVec2D.y * speed;
				moveVec.y = value;

				if (ticks >= delay && counter69 >= 5) { ticks = 0; player->lerpMotion(moveVec); }
				//if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
			}
			else MoveUtil::stop(false);
		}
	}

	//HiveTest by DarkNBTHax / Hive
	if (mode.getSelectedValue() == 1) {
		if (counter69 <= 20) {
			if (pressed) {
				//player->jumpFromGround();
				moveVec.x = moveVec2D.x * 0.85; moveVec.z = moveVec2D.y * 0.85;
				moveVec.y = -0.01f;

				if (ticks >= 6 && counter69 >= 5) { ticks = 0; player->lerpMotion(moveVec); }
				//if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
			}
			else MoveUtil::stop(false);
		}
	}

	//HiveTest by DarkNBTHax / HiveSafe
	if (mode.getSelectedValue() == 2) {
		if (counter69 <= 20) {
			if (pressed) {
				//player->jumpFromGround();
				moveVec.x = moveVec2D.x * 0.75; moveVec.z = moveVec2D.y * 0.75;
				moveVec.y = -0.05f;

				if (ticks >= 6 && counter69 >= 5) { ticks = 0; player->lerpMotion(moveVec); }
				//if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
			}
			else MoveUtil::stop(false);
		}
		else {
			MoveUtil::stop(false);
		}
	}

	//HiveClip by DarkNBTHax
	if (mode.getSelectedValue() == 4) {
		if (pressed) {
			moveVec.x = moveVec2D.x * 0.85; moveVec.z = moveVec2D.y * 0.85;
			moveVec.y = -0.01f;

			float clipvalue = player->getPos()->y;
			vec3_t clipPos = *player->getPos();
			player->setPos(vec3_t(clipPos.x, flypos.y + 1, clipPos.z));
			/*if (flypos.y - clipvalue > speed) {
				vec3_t clipPos = *player->getPos();
				clipvalue = flypos.y - clipvalue;
				player->setPos(vec3_t(clipPos.x, clipPos.y + clipvalue, clipPos.z));
			}*/


			//if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
		}
		else MoveUtil::stop(false);
	}

	//HiveClip by GrassCutterClient
	if (mode.getSelectedValue() == 4) {
		if (pressed) {
			moveVec.x = moveVec2D.x * 0.85; moveVec.z = moveVec2D.y * 0.85;
			moveVec.y = -0.01f;

			float clipvalue2 = player->getPos()->y;
			vec3_t clipPos2 = *player->getPos();
			float upvalue = speed;
			if (upvalue < player->fallDistance) {
				player->setPos(vec3_t(clipPos2.x, flypos.y, clipPos2.z));
				player->fallDistance = 0;
				player->velocity.y = 0.f;
			}
			//MoveUtil::setSpeed(friction);
			friction *= 0.97;
			//player->setPos(vec3_t(clipPos.x, flypos.y + 1, clipPos.z));
			/*if (flypos.y - clipvalue > speed) {
				vec3_t clipPos = *player->getPos();
				clipvalue = flypos.y - clipvalue;
				player->setPos(vec3_t(clipPos.x, clipPos.y + clipvalue, clipPos.z));
			}*/


			//if (enabledTick >= 5 && pressed && !player->onGround) player->lerpMotion(moveVec);
		}
		else MoveUtil::stop(false);
	}
}

void HiveFly::onDisable() {
	C_LocalPlayer* player = g_Data.getLocalPlayer();
	counter69 = 0;
	counter = 0;
	if (player != nullptr) {
		//player->velocity.x = 0.f;
		//player->velocity.y = 0.f;
		//player->velocity.z = 0.f;
		MoveUtil::stop(false);
		g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
	}
}
