#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <queue>
#include <set>

#include "../PacketClient/Config/AccountInformation.h"
#include "../SDK/CChestBlockActor.h"
#include "../SDK/CClientInstance.h"
#include "../SDK/CGameMode.h"
#include "../SDK/CHIDController.h"
#include "../SDK/CMoveInputHandler.h"
#include "../SDK/CRakNetInstance.h"
#include "../Utils/SkinUtil.h"
#include "../Utils/TextFormat.h"
#include "SlimMem.h"
#include <string>

enum DATAPACKET_CMD : int {
	CMD_INIT = 0,
	CMD_PING,
	CMD_PONG,
	CMD_OPENBROWSER,
	CMD_FILECHOOSER,
	CMD_RESPONSE,
	CMD_FOLDERCHOOSER,  // sets permissions for a whole folder and sends the path over
	CMD_LOG
};

struct NetworkedData {
	unsigned int xorKey = 0;
	unsigned int localPlayerOffset = 0x94;  // Scrambled data
	bool dataSet = false;
};

struct InfoBoxData {
	bool isOpen = true;
	float fadeTarget = 1;
	float fadeVal = 0;
	float duration = 0;
	int colorR = 0;
	int colorG = 0;
	int colorB = 0;
	std::string title;
	std::string message;

	InfoBoxData(std::string title, std::string message) : title(title), message(message) {};

	void fade() {
		fadeVal = fadeTarget - ((fadeTarget - fadeVal) + 0.65f);
		if (fadeTarget == 0 && fadeVal < 0.001f)
			isOpen = false;
	}
};

struct SkinData;

class GameData {
private:
	C_ClientInstance* clientInstance = nullptr;
	C_LocalPlayer* localPlayer = nullptr;
	C_GameMode* gameMode = nullptr;
	C_EntityList* entityList = nullptr;
	C_RakNetInstance* raknetInstance = nullptr;
	void* hDllInst = 0;
	std::vector<std::shared_ptr<AABB>> chestList;
	std::vector<std::string> textPrintList;
	std::mutex textPrintLock;
	std::mutex chestListMutex;
	int lastRequestId = 0;
	std::shared_ptr<std::string> customGeometry;
	bool customGeoActive = false;
	std::shared_ptr<std::tuple<std::shared_ptr<unsigned char[]>, size_t>> customTexture;
	bool customTextureActive = false;
	std::vector<std::shared_ptr<InfoBoxData>> infoBoxQueue;

	bool injectorConnectionActive = false;
	const SlimUtils::SlimModule* gameModule = 0;
	SlimUtils::SlimMem* slimMem;
	bool shouldTerminateB = false;
	bool shouldHideB = false;
	bool isAllowingWIPFeatures = false;
	__int64 lastUpdate;
	static void retrieveClientInstance();
	TextHolder* fakeName;

public:
	NetworkedData networkedData;
	C_HIDController* hidController = nullptr;

	static bool canUseMoveKeys();
	static bool isKeyDown(int key);
	static bool isKeyPressed(int key);
	static bool isRightClickDown();
	static bool isLeftClickDown();
	static bool isWheelDown();
	static bool shouldTerminate();
	static bool shouldHide();
	static void hide();
	static void terminate();
	static void updateGameData(C_GameMode* gameMode);
	static void initGameData(const SlimUtils::SlimModule* gameModule, SlimUtils::SlimMem* slimMem, void* hDllInst);
	static void addChestToList(C_ChestBlockActor* ChestBlock2);
	static void EntityList_tick(C_EntityList* list);
	static void setHIDController(C_HIDController* Hid);
	static void setRakNetInstance(C_RakNetInstance* raknet);
	static void log(const char* fmt, ...);
	float fov = 0.f;
	int fps = 0;
	int frameCount = 0;
	int cpsLeft = 0;
	int cpsRight = 0;
	int leftclickCount = 0;
	int rightclickCount = 0;



	void clearChestList() {
		std::lock_guard<std::mutex> listGuard(chestListMutex);
		this->chestList.clear();
	}

	inline std::shared_ptr<InfoBoxData> getFreshInfoBox() {
		while (!this->infoBoxQueue.empty()) {
			auto box = this->infoBoxQueue.front();
			if (!box->isOpen) {
				this->infoBoxQueue.erase(this->infoBoxQueue.begin());
				continue;
			}
			return box;
		}
		return std::shared_ptr<InfoBoxData>();
	}

	inline std::vector<std::shared_ptr<InfoBoxData>>& getInfoBoxList() {
		while (!this->infoBoxQueue.empty()) {
			auto box = this->infoBoxQueue.front();
			if (!box->isOpen) {
				this->infoBoxQueue.erase(this->infoBoxQueue.begin());
				continue;
			}
			break;
		}
		return this->infoBoxQueue;
	}

	inline std::shared_ptr<InfoBoxData> addNotification(std::string title, std::string message) { // success, warning, error
		auto box = std::make_shared<InfoBoxData>(title, message);
		this->infoBoxQueue.push_back(box);
		return box;
	}

	inline void setCustomGeometryOverride(bool setActive, std::shared_ptr<std::string> customGeoPtr) {
		this->customGeoActive = setActive;
		if (setActive)
			this->customGeometry.swap(customGeoPtr);
		else
			this->customGeometry.reset();
	}
	inline std::tuple<bool, std::shared_ptr<std::string>> getCustomGeoOverride() {
		return std::make_tuple(this->customGeoActive, this->customGeometry);
	}
	inline void setCustomTextureOverride(bool setActive, std::shared_ptr<std::tuple<std::shared_ptr<unsigned char[]>, size_t>> customTexturePtr) {
		this->customTextureActive = setActive;
		if (setActive)
			this->customTexture.swap(customTexturePtr);
		else
			this->customTexture.reset();
	}
	inline auto getCustomTextureOverride() {
		return std::make_tuple(this->customTextureActive, this->customTexture);
	}
	
	inline void setAllowWIPFeatures(bool enable = false) { isAllowingWIPFeatures = enable; };
	inline bool isInjectorConnectionActive() { return injectorConnectionActive; };
	inline void* getDllModule() { return hDllInst; };
	inline C_ClientInstance* getClientInstance() { return clientInstance; };
	inline C_GuiData* getGuiData() { return clientInstance->getGuiData(); };
	inline C_LocalPlayer* getLocalPlayer() {
		localPlayer = clientInstance->getLocalPlayer();
		if (localPlayer == nullptr)
			gameMode = nullptr;
		return localPlayer;
	};
	C_LocalPlayer** getPtrLocalPlayer() {
		return &localPlayer;
	};

	bool isInGame() {
		return localPlayer != nullptr;
	}

	const SlimUtils::SlimModule* getModule() {
		return gameModule;
	};
	const SlimUtils::SlimMem* getSlimMem() {
		return slimMem;
	};
	C_GameMode* getCGameMode() { return gameMode; };
	C_EntityList* getEntityList() { return entityList; };
	C_HIDController** getHIDController() { return &hidController; };
	C_RakNetInstance* getRakNetInstance() { return raknetInstance; };
	std::vector<std::shared_ptr<AABB>>* getChestList() { return &chestList; };
	auto lockChestList() { return std::lock_guard<std::mutex>(this->chestListMutex); }
	void setFakeName(TextHolder* name) { fakeName = name; };
	TextHolder* getFakeName() { return fakeName; };
	inline __int64 getLastUpdateTime() { return lastUpdate; };
	void forEachEntity(std::function<void(C_Entity*, bool)>);
	int getFPS() { return fps; };
	int getLeftCPS() { return cpsLeft; };
	int getRightCPS() { return cpsRight; };
};

extern GameData g_Data;