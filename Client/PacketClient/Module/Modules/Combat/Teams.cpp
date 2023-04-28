#include "Teams.h"
#include "../pch.h"

using namespace std;
Teams::Teams() : IModule(0, Category::COMBAT, "Automatically friends everyone on your team") {
	registerEnumSetting("Server", &mode, 0);
	mode.addEntry("Normal", 0);
	mode.addEntry("Hive", 1);
	registerBoolSetting("Allied", &alliedCheck, alliedCheck);
	registerBoolSetting("Color", &colorCheck, colorCheck);
}

const char* Teams::getModuleName() {
	return ("Teams");
}

static vector<C_Entity*> targetList;
void findEntTeams(C_Entity* currentEntity, bool isRegularEntity) {

	if (currentEntity == nullptr)
		return;

	if (currentEntity == g_Data.getLocalPlayer())
		return;

	//if (!g_Data.getLocalPlayer()->canAttack(currentEntity, false))
		//return;

	if (!g_Data.getLocalPlayer()->isAlive())
		return;

	if (!currentEntity->isAlive())
		return;

	if (currentEntity->getEntityTypeId() == 80 || currentEntity->getEntityTypeId() == 69)  // XP and Arrows
		return;

	//if (!TargetUtil::isValidTarget(currentEntity))
		//return;

	if (currentEntity->getEntityTypeId() == 51 || currentEntity->getEntityTypeId() == 1677999)  // Villagers and NPCS
		return;

	float dist = (*currentEntity->getPos()).dist(*g_Data.getLocalPlayer()->getPos());
	if (dist < 12) targetList.push_back(currentEntity);
}

void Teams::onEnable() {
	targetList.clear();
	SearchTeams = false;
	RemoveTeams = false;
}

void Teams::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	targetList.clear();
	g_Data.forEachEntity(findEntTeams);

	if (SearchTeams) {
		string tempname;
		for (auto& i : targetList) {
			string teamname = i->getNameTag()->getText();
			//teamname = Utils::sanitize(teamname);
			//teamname = teamname.substr(0, teamname.find('\n'));
			FriendList::addPlayerToList(teamname);
			tempname = teamname;
		}
		auto notififcation = g_Data.addNotification("Teams:", "Found team"); notififcation->duration = 5;
		SearchTeams = false;
	}

	if (RemoveTeams) {
		static auto friendlist = moduleMgr->getModule<FriendList>();
		FriendList::ClearFriend();
		//clientMessageF("ClearedTeams");
		RemoveTeams = false;
	}
}
