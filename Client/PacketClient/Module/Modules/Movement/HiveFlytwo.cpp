#include "HiveFlytwo.h"
HiveFlytwo::HiveFlytwo() : IModule(0, Category::MOVEMENT, "How the fuck does this bypass ?!?!?") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Jump", 0);
	mode.addEntry("Clip", 1);
	mode.addEntry("SlowClip1", 2);
	mode.addEntry("SlowClip2", 3);
	mode.addEntry("JumpClip", 4);
	mode.addEntry("SlowUp", 5);
	registerFloatSetting("Speed", &speed, speed, .1f, 2.f);
	registerFloatSetting("Height", &height, height, 0.f, 1.f);
	registerFloatSetting("Duration", &duration, duration, 0.5f, 1.f);
	registerFloatSetting("ClipUp", &clipUp, clipUp, 0.f, 5.f);
	registerIntSetting("Timer", &timer, timer, 1, 30);
	registerIntSetting("DashTime", &dashTime, dashTime, 0, 2000);
	registerBoolSetting("Freelook", &lock, lock);
	registerBoolSetting("ClipLimit", &cliplimit, cliplimit);
	registerIntSetting("ClipTimes", &cliptimes, cliptimes, 1, 20);
	registerFloatSetting("ClipValue", &clipvalue, clipvalue, 0.f, 2.f);
	registerFloatSetting("LerpSpeed", &upsped, upsped, 1, 4);
	registerFloatSetting("UpHeight", &upheight, upheight, 0.f, 3.f);
}

const char* HiveFlytwo::getRawModuleName() {
	return "HiveFly2";
}

const char* HiveFlytwo::getModuleName() {
	//name = string("HiveFlyTwo ") + string(GRAY) + mode.GetEntry(mode.getSelectedValue()).GetName();
	return name.c_str();
}


void HiveFlytwo::onEnable() {
	dashed = false;
	dspeed = speed;
	nowtimes = 0;
	auto player = g_Data.getLocalPlayer();
	savePos = *player->getPos();
	vec3_t myPos = *player->getPos();
	if (mode.getSelectedValue() == 0 || mode.getSelectedValue() == 5) // only jump
	{
		myPos.y += clipUp;
		player->setPos(myPos);
	}
	if (lock)
	{
		auto lock = moduleMgr->getModule<Freelook>();
		lock->setEnabled(true);
	}
	auto aura = moduleMgr->getModule<Killaura>();
	auto disa = moduleMgr->getModule<Disabler>();
	auto sped = moduleMgr->getModule<Speed>();
	if (aura->isEnabled())
	{
		aurais = true;
		aura->setEnabled(false);
	}
	else aurais = false;
	if (disa->isEnabled())
	{
		disableris = true;
		disa->setEnabled(false);
	}
	else disableris = false;
	if (sped->isEnabled())
	{
		speedis = true;
		sped->setEnabled(false);
	}
	else speedis = false;
	g_Data.getClientInstance()->minecraft->setTimerSpeed(timer);
}

void HiveFlytwo::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
}

void HiveFlytwo::onMove(C_MoveInputHandler* input) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	vec3_t moveVec;
	auto sped = moduleMgr->getModule<Speed>();
	bool blink = false;
	if (g_Data.canUseMoveKeys()) {
		if (sped->isEnabled())
		{
			speedis = true;
			sped->setEnabled(false);
		}
		else speedis = false;
		if (mode.getSelectedValue() == 2 || mode.getSelectedValue() == 3)
		{
			if (!aids)
			{
				vec3_t checkPos = *player->getPos(); //antikick
				if (checkPos.y > savePos.y + 0.01)
				{
					player->velocity.y = 0;
					vec3_t nowPos = *player->getPos();
					savePos.y -= clipvalue;
					nowPos.y = savePos.y - clipvalue;
					if (mode.getSelectedValue() == 2) player->setPos(nowPos);
					if (mode.getSelectedValue() == 3) aids = true;
					MoveUtil::setSpeed(speed);
				}
			}
		}
		if (mode.getSelectedValue() == 5)
		{
			vec3_t checkPos = *player->getPos();
			if (!player->fallDistance >= 0.1)
			{
				
			}
			if (checkPos.y <= savePos.y)
			{
				player->velocity.x = 0;
				player->velocity.z = 0;
				moveVec.x = 0;
				moveVec.y = height * upsped;
				moveVec.z = 0;
				player->lerpMotion(moveVec);
				aids = false;
			}
			if (checkPos.y >= savePos.y + upheight)
			{
				if (!aids)
				{
					player->velocity.y = 0;
					MoveUtil::setSpeed(speed);
					aids = true;
				}
			}
			else
			{
				if (player->velocity.y >= 0.01)
				{
					player->velocity.x = 0;
					player->velocity.z = 0;
				}
			}
		}
		if (TimerUtil::hasTimedElapsed(dashTime, !blink) && !dashed) {
			dashed = true;
			dspeed = dspeed * duration;
			if(mode.getSelectedValue() == 0) // clip, slowclip, jumpclip
			{
				float calcYaw = (player->yaw + 90) * (PI / 180);
				vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
				bool pressed = moveVec2d.magnitude() > 0.01f;
				float c = cos(calcYaw);
				float s = sin(calcYaw);
				moveVec2d = { moveVec2d.x * c - moveVec2d.y * s, moveVec2d.x * s + moveVec2d.y * c };

				if (player->onGround && pressed)
					player->jumpFromGround();
				moveVec.x = moveVec2d.x * dspeed;
				moveVec.y = height;
				player->velocity.y;
				moveVec.z = moveVec2d.y * dspeed;
				if (pressed) player->lerpMotion(moveVec);
			}
			else
			{
				if (mode.getSelectedValue() == 1 || mode.getSelectedValue() == 2 || mode.getSelectedValue() == 3 || mode.getSelectedValue() == 4)
				{
					aids = false;
					if (cliplimit)
					{
						if (nowtimes > cliptimes)
						{
							auto two = moduleMgr->getModule<HiveFlytwo>();
							two->setEnabled(false);
						}
						else
						{
							{
								nowtimes++;
								vec3_t nowPos = *player->getPos();
								savePos.y -= clipvalue;
								if (mode.getSelectedValue() == 1 || mode.getSelectedValue() == 4)
								{
									MoveUtil::setSpeed(dspeed);
									nowPos.y = savePos.y;
									player->setPos(nowPos); //normal
									player->velocity.y = 0;
								}
								if (mode.getSelectedValue() == 2 || mode.getSelectedValue() == 3) //slow
								{
									moveVec.x = 0;
									moveVec.y = (savePos.y - nowPos.y) / (5 - upsped);
									moveVec.z = 0;
									player->lerpMotion(moveVec);
								}
								if (mode.getSelectedValue() == 4) // jump
								{
									float calcYaw = (player->yaw + 90) * (PI / 180);
									vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
									bool pressed = moveVec2d.magnitude() > 0.01f;
									float c = cos(calcYaw);
									float s = sin(calcYaw);
									moveVec2d = { moveVec2d.x * c - moveVec2d.y * s, moveVec2d.x * s + moveVec2d.y * c };

									if (player->onGround && pressed)
										player->jumpFromGround();
									moveVec.x = moveVec2d.x * dspeed;
									moveVec.y = height;
									player->velocity.y;
									moveVec.z = moveVec2d.y * dspeed;
									if (pressed) player->lerpMotion(moveVec);
								}
							}
						}
					}
					else
					{
						vec3_t nowPos = *player->getPos();
						savePos.y -= clipvalue;
						if (mode.getSelectedValue() == 1 || mode.getSelectedValue() == 4)
						{
							MoveUtil::setSpeed(dspeed);
							nowPos.y = savePos.y;
							player->setPos(nowPos);
							player->velocity.y = 0;
						}
						if (mode.getSelectedValue() == 2 || mode.getSelectedValue() == 3)
						{
							moveVec.x = 0;
							moveVec.y = (savePos.y - nowPos.y) / (5 - upsped);
							moveVec.z = 0;
							player->lerpMotion(moveVec);
						}
						if (mode.getSelectedValue() == 4)
						{
							float calcYaw = (player->yaw + 90) * (PI / 180);
							vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
							bool pressed = moveVec2d.magnitude() > 0.01f;
							float c = cos(calcYaw);
							float s = sin(calcYaw);
							moveVec2d = { moveVec2d.x * c - moveVec2d.y * s, moveVec2d.x * s + moveVec2d.y * c };

							if (player->onGround && pressed)
								player->jumpFromGround();
							moveVec.x = moveVec2d.x * dspeed;
							moveVec.y = height;
							player->velocity.y;
							moveVec.z = moveVec2d.y * dspeed;
							if (pressed) player->lerpMotion(moveVec);
						}
					}
				}
			}
		}
		else {
			dashed = false;
		}
	}
}

void HiveFlytwo::onDisable() {
	uintptr_t ViewBobbing = FindSignature("0F B6 80 DB 01 00 00");
	Utils::patchBytes((unsigned char*)ViewBobbing, (unsigned char*)"\x0F\xB6\x80\xDB\x01\x00\x00", 7);
	g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
	auto speedMod = moduleMgr->getModule<Speed>();
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;
	MoveUtil::fullStop(false);
	if (lock)
	{
		auto lock = moduleMgr->getModule<Freelook>();
		lock->setEnabled(false);
	}
	auto aura = moduleMgr->getModule<Killaura>();
	auto disa = moduleMgr->getModule<Disabler>();
	auto sped = moduleMgr->getModule<Speed>();
	if (aurais == true)
	{
		aura->setEnabled(true);
		aurais = false;
	}
	if (disableris == true)
	{
		disa->setEnabled(true);
		disableris = false;
	}
	if (speedis == true)
	{
		sped->setEnabled(true);
		speedis = false;
	}
}
