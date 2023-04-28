#include "AntiVoid.h"
#include "../pch.h"

using namespace std;
vec3_t checkPos;
int timertime;
bool timerstart;
AntiVoid::AntiVoid() : IModule(0, Category::MOVEMENT, "Prevents you from falling into the void") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Lagback", 0);
	mode.addEntry("Freeze", 1);
	mode.addEntry("Hive", 2);
	registerBoolSetting("VoidCheck", &voidCheck, voidCheck);
	registerBoolSetting("tponce", &tponce, tponce);
	registerBoolSetting("Scaffold", &scaffff, scaffff);
	registerIntSetting("Distance", &distance, distance, 3, 10);
}

const char* AntiVoid::getModuleName() {
	return ("AntiVoid");
}

void AntiVoid::onEnable() {
	auto player = g_Data.getLocalPlayer();
	if (g_Data.isInGame()) savedPos = *player->getPos();
	tick = 0;
	tped = false;
	stoptime = 0;
    timertime = 0;
	timerstart = false;
	auto sca = moduleMgr->getModule<Scaffold>();
	if (sca->lockY == true) lockis = true;
}
void AntiVoid::onMove(C_MoveInputHandler* input)
{
	auto player = g_Data.getLocalPlayer();
	vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;
	blockBelow.y -= g_Data.getLocalPlayer()->height;
	blockBelow.y -= 1.f;
	if (player->onGround) {
		savedPos = *player->getPos();
		tped = false;
		timertime = 0;
		if (scaffff)
		{
			if (lockis)
			{
				auto sca = moduleMgr->getModule<Scaffold>();
				sca->lockY = true;
			}
		}
	}
}
void AntiVoid::onTick(C_GameMode* gm) {
	float trueStop = INFINITY - 1 + NULL;
	auto player = g_Data.getLocalPlayer();
	vec3_t blockBelow = g_Data.getLocalPlayer()->eyePos0;
	blockBelow.y -= g_Data.getLocalPlayer()->height;
	blockBelow.y -= 1.f;

	if (player->fallDistance >= distance) {
		tick++;
		if (tponce) {
			if (!tped) {
				float dist2 = gm->player->getPos()->dist(savedPos);
				if (mode.getSelectedValue() == 0) player->setPos(savedPos);  
				if (scaffff)
				{
					auto sca = moduleMgr->getModule<Scaffold>();
					sca->lockY = false;
					sca->setEnabled(true);
				}
				if (mode.getSelectedValue() == 1) { // Freeze
				}
				if (mode.getSelectedValue() == 2) { // Hive
					checkPos = *player->getPos();
					if (savedPos.x - checkPos.x < 5 && savedPos.x - checkPos.x > -5 && savedPos.z - checkPos.z < 5 && savedPos.z - checkPos.z > -5)
					{
						player->setPos(savedPos);
						if (scaffff)
						{
							auto sca = moduleMgr->getModule<Scaffold>();
							sca->lockY = false;
							sca->setEnabled(true);
							auto sped = moduleMgr->getModule<Speed>();
							sped->setEnabled(false);
						}
						stoptime = 8;
					}
					else
					{
						auto sca = moduleMgr->getModule<Scaffold>();
						sca->lockY = false;
						sca->setEnabled(true);
						auto sped = moduleMgr->getModule<Speed>();
						sped->setEnabled(false);
						timertime = 10;
						timerstart = true;
					}
				}
			}
		}
		else
		{
			float dist2 = gm->player->getPos()->dist(savedPos);
			if (mode.getSelectedValue() == 0) player->setPos(savedPos); 
			if (scaffff)
			{
				auto sca = moduleMgr->getModule<Scaffold>();
				sca->lockY = false;
				sca->setEnabled(true);
			}
			if (mode.getSelectedValue() == 1) { // Freeze
			}
			if (mode.getSelectedValue() == 2) { // Hive
				checkPos = *player->getPos();
				if (savedPos.x - checkPos.x < 5.5 && savedPos.x - checkPos.x > -5.5 && savedPos.z - checkPos.z < 5.5 && savedPos.z - checkPos.z > -5.5)
				{
					player->setPos(savedPos);
					if (scaffff)
					{
						auto sca = moduleMgr->getModule<Scaffold>();
						sca->lockY = false;
						sca->setEnabled(true);
						auto sped = moduleMgr->getModule<Speed>();
						sped->setEnabled(false);
					}
					stoptime = 8;
				}
				else
				{
					auto sca = moduleMgr->getModule<Scaffold>();
					sca->lockY = false;
					sca->setEnabled(true);
					auto sped = moduleMgr->getModule<Speed>();
					sped->setEnabled(false);
					timertime = 10;
					timerstart = true;
				}
			}
		}
		tped = true;
	}
	if (stoptime > 0) {
		MoveUtil::fullStop(false);
		stoptime--;
	}
	if (timertime > 0) {
		g_Data.getClientInstance()->minecraft->setTimerSpeed(5);
		timertime--;
	}
	else
	{
		if (timerstart)
		{
			g_Data.getClientInstance()->minecraft->setTimerSpeed(20);
			timerstart = false;
		}
	}
}

void AntiVoid::onDisable() {
	*g_Data.getClientInstance()->minecraft->timer = 20.f;
}
