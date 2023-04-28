#include "CEntity.h"
#include "../Memory/GameData.h"
#include "../Utils/Utils.h"

C_InventoryTransactionManager* C_Entity::getTransactionManager() {
	static unsigned int offset = 0x1210;
	return reinterpret_cast<C_InventoryTransactionManager*>(reinterpret_cast<__int64>(this) + offset);
}

C_PlayerInventoryProxy* C_Player::getSupplies() {
	static unsigned int offset = 0xB70;
	/*if (offset == 0) {
		offset = *reinterpret_cast<int *>(FindSignature("48 8B 51 ?? 4C 8B 82 ?? ?? ?? ?? 48 8B B2 ?? ?? ?? ?? 41 80 B8") + 7);  // GameMode::startDestroyBlock -> GameMode::_canDestroy -> getSupplies
	}*/
	return *reinterpret_cast<C_PlayerInventoryProxy**>(reinterpret_cast<__int64>(this) + offset);
}

void C_LocalPlayer::unlockAchievements() {  // MinecraftEventing::fireEventAwardAchievement
	using fireEventAward = void(__fastcall*)(void*, int);
	static fireEventAward fireEventAwardFunc = reinterpret_cast<fireEventAward>(FindSignature("48 85 C9 0F 84 ? ? ? ? 48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B 01"));
	for (int i = 0; i < 118; i++)
		fireEventAwardFunc(this, i);
}

void C_LocalPlayer::applyTurnDelta(vec2_t* viewAngleDelta) {
	using applyTurnDelta = void(__thiscall*)(void*, vec2_t*);
	static applyTurnDelta TurnDelta = reinterpret_cast<applyTurnDelta>(FindSignature("48 8B C4 48 89 58 18 48 89 68 20 56 57 41 56 48 81 EC ?? ?? ?? ?? 0F 29 70 D8 0F 29 78 C8 44 0F 29 40 ?? 48 8B 05 ?? ?? ?? ??"));
	TurnDelta(this, viewAngleDelta);
}

void C_LocalPlayer::setGameModeType(int gma) {
	this->setC_PlayerGameType(gma);
}

bool PointingStruct::hasEntity() {
	return rayHitType == 1;
}

float C_Entity::getBlocksPerSecond() {
	return getTicksPerSecond() * *g_Data.getClientInstance()->minecraft->timer;
}

C_Entity* PointingStruct::getEntity() {
	if (rayHitType != 1) return nullptr;
	C_Entity* retval = nullptr;
	g_Data.forEachEntity([this, &retval](C_Entity* ent, bool b) {
		if (*(__int64*)((__int64)ent + 0x10) == GamingEntityFinder) {
			retval = ent;
			return;
		}
		});
	return retval;
}