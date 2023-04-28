#include "Module.h"
#include "../../../Utils/Json.hpp"
#include "../../../Utils/Logger.h"
#include <cstdarg>
#include "../../../Memory/Hooks.h"
#include <iomanip>
#include <chrono>
#include <string>
#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <direct.h>
#include <conio.h>
#include <sstream>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include<TlHelp32.h>
#include <tchar.h> 
#include <vector>
#include <thread>
#include <wtsapi32.h>
#include <psapi.h>
#include <math.h>
#include <algorithm> 
#include <playsoundapi.h>
#include<windows.h>
#include <tchar.h>
#include <urlmon.h>
#include "pch.h"

using json = nlohmann::json;
using namespace std;

#pragma region EnumEntry
EnumEntry::EnumEntry(const string _name, const unsigned char value) {
	name = _name;
	val = value;
}

EnumEntry::EnumEntry(const char* _name, const unsigned char value) {
	name = _name;
	val = value;
}

string EnumEntry::GetName() {
	return name;
}

unsigned char EnumEntry::GetValue() {
	return val;
}

#pragma endregion
#pragma region SettingEnum
SettingEnum::SettingEnum(vector<EnumEntry>& entr, IModule* mod) {
	Entrys = entr;
	owner = mod;
	sort(Entrys.begin(), Entrys.end(), [](EnumEntry rhs, EnumEntry lhs) {
		return rhs.GetValue() < lhs.GetValue();
		});
}

SettingEnum::SettingEnum(IModule* mod) {
	owner = mod;
}

SettingEnum& SettingEnum::addEntry(EnumEntry entr) {
	auto etr = EnumEntry(entr);
	bool SameVal = false;
	for (auto it = this->Entrys.begin(); it != this->Entrys.end(); it++) {
		SameVal |= it->GetValue() == etr.GetValue();
	}
	if (!SameVal) {
		Entrys.push_back(etr);
		sort(Entrys.begin(), Entrys.end(), [](EnumEntry rhs, EnumEntry lhs) {
			return rhs.GetValue() < lhs.GetValue();
			});
	}
	return *this;
}

SettingEnum& SettingEnum::addEntry(const char* name, int value) {
	auto etr = EnumEntry(name, value);
	bool SameVal = false;
	for (auto it : Entrys)
		SameVal |= it.GetValue() == etr.GetValue();

	if (!SameVal) {
		Entrys.push_back(etr);
		sort(Entrys.begin(), Entrys.end(), [](EnumEntry rhs, EnumEntry lhs) {
			return rhs.GetValue() < lhs.GetValue();
			});
	}
	return *this;
}

EnumEntry& SettingEnum::GetEntry(int ind) {
	return Entrys.at(ind);
}

EnumEntry& SettingEnum::GetSelectedEntry() {
	return GetEntry(selected);
}

int SettingEnum::getSelectedValue() {
	return GetEntry(selected).GetValue();
}

int SettingEnum::GetCount() {
	return (int)Entrys.size();
}

int SettingEnum::SelectNextValue(bool back) {
	if (back)
		selected--;
	else
		selected++;

	if (Entrys.size() <= selected) {
		selected = 0;
		return selected;
	}
	else if (selected < 0) {
		selected = (int)Entrys.size() - 1;
		return selected;
	}
	return selected;
}
#pragma endregion

IModule::IModule(int key, Category c, const char* tooltip) {
	this->keybind = key;
	this->category = c;
	this->tooltip = tooltip;
	this->registerKeybindSetting(std::string("Keybind"), &this->keybind, this->keybind);
	this->registerBoolSetting(string("enabled"), &this->enabled, false);
	this->registerBoolSetting(string("Hide in list"), &this->shouldHide, false);
	this->ModulePos = vec2_t(0.f, 0.f);
}

void IModule::registerFloatSetting(std::string name, float* floatPtr, float defaultValue, float minValue, float maxValue) {
#ifdef DEBUG
	if (minValue > maxValue)
		__debugbreak();  // Minimum value is bigger than maximum value
#endif

	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::FLOAT_T;

	setting->value = reinterpret_cast<SettingValue*>(floatPtr);

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_float = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value
	SettingValue* minVal = new SettingValue();
	minVal->_float = minValue;
	setting->minValue = minVal;

	// Max Value
	SettingValue* maxVal = new SettingValue();
	maxVal->_float = maxValue;
	setting->maxValue = maxVal;

	strcpy_s(setting->name, 19, name.c_str());  // Name

	settings.push_back(setting);  // Add to list
}

SettingEntry* IModule::registerIntSetting(std::string name, int* intPtr, int defaultValue, int minValue, int maxValue) {
#ifdef DEBUG
	if (minValue > maxValue)
		__debugbreak();  // Minimum value is bigger than maximum value
#endif

	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::INT_T;
	setting->value = reinterpret_cast<SettingValue*>(intPtr);  // Actual Value

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value
	SettingValue* minVal = new SettingValue();
	minVal->_int = minValue;
	setting->minValue = minVal;

	// Max Value
	SettingValue* maxVal = new SettingValue();
	maxVal->_int = maxValue;
	setting->maxValue = maxVal;

	// Name
	strcpy_s(setting->name, 19, name.c_str());

	settings.push_back(setting);  // Add to list
	return setting;
}

void IModule::registerEnumSetting(std::string name, SettingEnum* ptr, int defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::ENUM_T;
	if (defaultValue < 0 || defaultValue >= ptr->GetCount())
		defaultValue = 0;

	// Actual Value
	setting->value = reinterpret_cast<SettingValue*>(&ptr->selected);
	setting->value->_int = defaultValue;

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value (is Extended)
	SettingValue* minVal = new SettingValue();
	minVal->_bool = false;
	setting->minValue = minVal;

	// Enum data
	setting->extraData = ptr;

	strcpy_s(setting->name, 19, name.c_str());
	settings.push_back(setting);
}

SettingEntry* IModule::registerEnumSettingGroup(std::string name, SettingEnum* ptr, int defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::ENUM_SETTING_GROUP_T;
	if (defaultValue < 0 || defaultValue >= ptr->GetCount())
		defaultValue = 0;

	// Actual Value
	setting->value = reinterpret_cast<SettingValue*>(&ptr->selected);
	setting->value->_int = defaultValue;

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value (is Extended)
	SettingValue* minVal = new SettingValue();
	minVal->_bool = false;
	setting->minValue = minVal;

	// Enum data
	setting->extraData = ptr;

	strcpy_s(setting->name, 19, name.c_str());

	int numToAdd = ptr->Entrys.size();

	while (numToAdd > 0) {
		setting->groups.push_back(nullptr);
		numToAdd--;
	}

	settings.push_back(setting);

	return setting;
}

void IModule::registerBoolSetting(std::string name, bool* boolPtr, bool defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::BOOL_T;

	setting->value = reinterpret_cast<SettingValue*>(boolPtr);  // Actual value

	SettingValue* defaultVal = new SettingValue();  // Default Value
	defaultVal->_bool = defaultValue;
	setting->defaultValue = defaultVal;

	strcpy_s(setting->name, 19, name.c_str());  // Name

	settings.push_back(setting);  // Add to list
}

SettingEntry* IModule::registerKeybindSetting(std::string name, int* intPtr, int defaultValue) {
	SettingEntry* newSetting = registerIntSetting(name, intPtr, defaultValue, 0, 0xFF);
	newSetting->valueType = ValueType::KEYBIND_T;

	return newSetting;
}

SettingEntry* SettingGroup::registerFloatSetting(std::string name, float* floatPtr, float defaultValue, float minValue, float maxValue) {
#ifdef DEBUG
	if (minValue > maxValue)
		__debugbreak();  // Minimum value is bigger than maximum value
#endif

	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::FLOAT_T;

	setting->value = reinterpret_cast<SettingValue*>(floatPtr);

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_float = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value
	SettingValue* minVal = new SettingValue();
	minVal->_float = minValue;
	setting->minValue = minVal;

	// Max Value
	SettingValue* maxVal = new SettingValue();
	maxVal->_float = maxValue;
	setting->maxValue = maxVal;

	strcpy_s(setting->name, 19, name.c_str());  // Name

	setting->nestValue = parent->nestValue + 1;

	entries.push_back(setting);  // Add to list

	return setting;
}

SettingEntry* SettingGroup::registerIntSetting(std::string name, int* intPtr, int defaultValue, int minValue, int maxValue) {
#ifdef DEBUG
	if (minValue > maxValue)
		__debugbreak();  // Minimum value is bigger than maximum value
#endif

	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::INT_T;
	setting->value = reinterpret_cast<SettingValue*>(intPtr);  // Actual Value

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value
	SettingValue* minVal = new SettingValue();
	minVal->_int = minValue;
	setting->minValue = minVal;

	// Max Value
	SettingValue* maxVal = new SettingValue();
	maxVal->_int = maxValue;
	setting->maxValue = maxVal;

	// Name
	strcpy_s(setting->name, 19, name.c_str());

	setting->nestValue = parent->nestValue + 1;

	entries.push_back(setting);  // Add to list
	return setting;
}

SettingEntry* SettingGroup::registerEnumSetting(std::string name, SettingEnum* ptr, int defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::ENUM_T;
	if (defaultValue < 0 || defaultValue >= ptr->GetCount())
		defaultValue = 0;

	// Actual Value
	setting->value = reinterpret_cast<SettingValue*>(&ptr->selected);
	setting->value->_int = defaultValue;

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value (is Extended)
	SettingValue* minVal = new SettingValue();
	minVal->_bool = false;
	setting->minValue = minVal;

	// Enum data
	setting->extraData = ptr;

	setting->nestValue = parent->nestValue + 1;

	strcpy_s(setting->name, 19, name.c_str());
	entries.push_back(setting);

	return setting;
}

SettingEntry* SettingGroup::registerEnumSettingGroup(std::string name, SettingEnum* ptr, int defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::ENUM_SETTING_GROUP_T;
	if (defaultValue < 0 || defaultValue >= ptr->GetCount())
		defaultValue = 0;

	// Actual Value
	setting->value = reinterpret_cast<SettingValue*>(&ptr->selected);
	setting->value->_int = defaultValue;

	// Default Value
	SettingValue* defaultVal = new SettingValue();
	defaultVal->_int = defaultValue;
	setting->defaultValue = defaultVal;

	// Min Value (is Extended)
	SettingValue* minVal = new SettingValue();
	minVal->_bool = false;
	setting->minValue = minVal;

	// Enum data
	setting->extraData = ptr;

	strcpy_s(setting->name, 19, name.c_str());

	int numToAdd = ptr->Entrys.size();

	while (numToAdd > 0) {
		setting->groups.push_back(nullptr);
		numToAdd--;
	}

	setting->nestValue = parent->nestValue + 1;

	entries.push_back(setting);

	return setting;
}

SettingEntry* SettingGroup::registerBoolSetting(std::string name, bool* boolPtr, bool defaultValue) {
	SettingEntry* setting = new SettingEntry();
	setting->valueType = ValueType::BOOL_T;

	setting->value = reinterpret_cast<SettingValue*>(boolPtr);  // Actual value

	SettingValue* defaultVal = new SettingValue();  // Default Value
	defaultVal->_bool = defaultValue;
	setting->defaultValue = defaultVal;

	strcpy_s(setting->name, 19, name.c_str());  // Name

	setting->nestValue = parent->nestValue + 1;

	entries.push_back(setting);  // Add to list

	return setting;
}

SettingEntry* SettingGroup::registerKeybindSetting(std::string name, int* intPtr, int defaultValue) {
	SettingEntry* newSetting = registerIntSetting(name, intPtr, defaultValue, 0, 0xFF);
	newSetting->valueType = ValueType::KEYBIND_T;

	return newSetting;
}
IModule::~IModule() {
	for (auto it = this->settings.begin(); it != this->settings.end(); it++) {
		delete* it;
	}
	this->settings.clear();
}

const char* IModule::getModuleName() {
	return "Module";
}

const char* IModule::getRawModuleName() {
	return getModuleName();
}

int IModule::getKeybind() {
	return this->keybind;
}

void IModule::setKeybind(int key) {
	this->keybind = key;
}

bool IModule::allowAutoStart() {
	return true;
}

void IModule::onPlayerTick(C_Player*) {
}

void IModule::onWorldTick(C_GameMode*) {
}

void IModule::onTick(C_GameMode*) {
}

void IModule::onKeyUpdate(int key, bool isDown) {
	if (!moduleMgr->getModule<ClickGUIMod>()->hasOpenedGUI) {
		if (key == getKeybind()) {
			if (isHoldMode())
				setEnabled(isDown);
			else if (isDown)
				toggle();
		}
	}
}

void IModule::onEnable() {
}

void IModule::onDisable() {
}

void IModule::onPreRender(C_MinecraftUIRenderContext* renderCtx) {
}

void IModule::onPostRender(C_MinecraftUIRenderContext* renderCtx) {
}

void IModule::onSendPacket(C_Packet*) {
}

#pragma warning(pop)

void IModule::onLoadSettings(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	if (conf->contains(this->getRawModuleName())) {
		auto obj = conf->at(this->getRawModuleName());
		if (obj.is_null())
			return;
		for (auto it = this->settings.begin(); it != this->settings.end(); ++it) {
			SettingEntry* sett = *it;
			if (obj.contains(sett->name)) {
				auto value = obj.at(sett->name);
				if (value.is_null())
					continue;
				try {
					switch (sett->valueType) {
					case ValueType::KEYBIND_T:
						sett->value->_int = value.get<int>();
						break;
					}
					//sett->makeSureTheKeyValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
					sett->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
					continue;
				}
				catch (exception e) {
					logF("Keybind Load Error (%s): %s", this->getRawModuleName(), e.what());
				}
			}
			if (obj.contains("Hidden")) {
				auto value = obj.at("Hidden");
				if (value.is_null())
					continue;
				shouldHide = value.get<bool>();
			}
		}

		for (vector<string>::iterator it = g_friend.List.begin(); it != g_friend.List.end(); ++it) {
			if (conf->contains("Friends")) {
				auto friends = conf->at("Friends");//e
				if (!friends.is_null()) {
					auto value = friends.at(it->data());
					string name = value.get < string >();
					name = Utils::sanitize(name);
					name = name.substr(0, name.find('\n'));
					if (!FriendList::findPlayer(name))
						FriendList::addPlayerToList(name);
				}
			}
		}
		if (this->enabled)
			this->onEnable();
	}
}

#pragma warning(push)
#pragma warning(disable : 26444)
void IModule::onSaveSettings(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	string modName = getRawModuleName();
	if (conf->contains(modName.c_str()))
		conf->erase(modName.c_str());

	json obj = {};
	//auto obj = conf->at(modName);
	for (auto sett : this->settings) {
		obj.emplace("Hidden", this->shouldHide);
		switch (sett->valueType) {
		case ValueType::KEYBIND_T:
			obj.emplace(sett->name, sett->value->_int);
			break;
		}
	}
	json Friends = {};
	for (vector<string>::iterator it = g_friend.List.begin(); it != g_friend.List.end(); ++it)
		Friends.emplace(it->data(), (int)it->length());

	conf->emplace(modName.c_str(), obj);
	conf->emplace("Friends", Friends);
}


#pragma warning(pop)

void IModule::onLoadConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	if (conf->contains(this->getRawModuleName())) {
		auto obj = conf->at(this->getRawModuleName());
		if (obj.is_null())
			return;
		for (auto it = this->settings.begin(); it != this->settings.end(); ++it) {
			SettingEntry* sett = *it;
			if (obj.contains(sett->name)) {
				auto value = obj.at(sett->name);
				if (value.is_null())
					continue;
				try {
					switch (sett->valueType) {
					case ValueType::FLOAT_T:
						sett->value->_float = value.get<float>();
						break;
					case ValueType::DOUBLE_T:
						sett->value->_double = value.get<double>();
						break;
					case ValueType::INT64_T:
						sett->value->int64 = value.get<__int64>();
						break;
					case ValueType::ENUM_T:
					case ValueType::KEYBIND_T:
					case ValueType::INT_T:
						sett->value->_int = value.get<int>();
						break;
					case ValueType::BOOL_T:
						sett->value->_bool = value.get<bool>();
						break;
					case ValueType::TEXT_T:
						sett->value->text = new std::string(value.get<std::string>());
						break;
					case ValueType::SETTING_GROUP_T: {
						json group = value.get<json>();
						sett->groups[0]->onLoadConfig(&group);
					} break;
					case ValueType::BOOL_SETTING_GROUP_T: {
						json obj2 = value.get<json>();
						if (obj2.contains("value") && !obj2.at("value").is_null()) {
							sett->value->_bool = obj2["value"].get<bool>();
						}
						if (obj2.contains("group") && obj2.at("group").is_object() && !obj.at("group").is_null()) {
							json group = obj2["group"].get<json>();

							sett->groups[0]->onLoadConfig(&group);
						}
					} break;
					case ValueType::ENUM_SETTING_GROUP_T: {
						json obj2 = value;
						if (obj2.contains("value") && !obj2.at("value").is_null()) {
							sett->value->_int = obj["value"].get<int>();
						}
						if (obj2.contains("groups") && obj2.at("groups").is_array() && !obj.at("group").is_null()) {
							std::vector<json> groups = obj2["groups"].get<std::vector<json>>();

							int i = 0;
							for (auto it2 = groups.begin(); it2 != groups.end(); ++it2, ++i) {
								if (i >= sett->groups.size())
									break;

								if (sett->groups[i] != nullptr) {
									sett->groups[i]->onLoadConfig(it2._Ptr);
								}
							}
						}
					} break;
					}
					sett->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
					continue;
				}
				catch (std::exception e) {
					logF("Config Load Error (%s): %s", this->getRawModuleName(), e.what());
				}
			}
		}
		if (this->enabled)
			this->onEnable();
	}
}

void SettingGroup::onLoadConfig(void* confVoid) {
	json& obj = *reinterpret_cast<json*>(confVoid);
	for (auto it = entries.begin(); it != entries.end(); ++it) {
		SettingEntry* sett = *it;
		if (obj.contains(sett->name)) {
			auto value = obj.at(sett->name);
			if (value.is_null())
				continue;
			try {
				switch (sett->valueType) {
				case ValueType::FLOAT_T:
					sett->value->_float = value.get<float>();
					break;
				case ValueType::DOUBLE_T:
					sett->value->_double = value.get<double>();
					break;
				case ValueType::INT64_T:
					sett->value->int64 = value.get<__int64>();
					break;
				case ValueType::ENUM_T:
				case ValueType::KEYBIND_T:
				case ValueType::INT_T:
					sett->value->_int = value.get<int>();
					break;
				case ValueType::BOOL_T:
					sett->value->_bool = value.get<bool>();
					break;
				case ValueType::TEXT_T:
					sett->value->text = new std::string(value.get<std::string>());
					break;
				case ValueType::SETTING_GROUP_T: {
					json group = value.get<json>();
					sett->groups[0]->onLoadConfig(&group);
				} break;
				case ValueType::BOOL_SETTING_GROUP_T: {
					json obj2 = value.get<json>();
					if (obj2.contains("value") && !obj2.at("value").is_null()) {
						sett->value->_bool = obj2["value"].get<bool>();
					}
					if (obj2.contains("group") && obj2.at("group").is_object() && !obj.at("group").is_null()) {
						json group = obj2["group"].get<json>();
						sett->groups[0]->onLoadConfig(&group);
					}
				} break;
				case ValueType::ENUM_SETTING_GROUP_T: {
					json obj2 = value;
					if (obj2.contains("value") && !obj2.at("value").is_null()) {
						sett->value->_int = obj["value"].get<int>();
					}
					if (obj2.contains("groups") && obj2.at("groups").is_object() && !obj.at("group").is_null()) {
						for (auto n = 0; n < sett->groups.size(); n++) {
							if (sett->groups[n] != nullptr && obj2.contains(std::to_string(n)) && !obj2.at(std::to_string(n)).is_null() && obj2.at(std::to_string(n)).is_object()) {
								sett->groups[n]->onLoadConfig(&obj2[std::to_string(n)]);
							}
						}
					}
				} break;
				}
				sett->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
				continue;
			}
			catch (std::exception e) {
				logF("Config Load Error in setting group #sad (%s): %s", parent->name, e.what());
			}
		}
	}
}

#pragma warning(push)
#pragma warning(disable : 26444)
void IModule::onSaveConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	std::string modName = getRawModuleName();
	if (conf->contains(modName.c_str()))
		conf->erase(modName.c_str());

	json obj = {};
	//auto obj = conf->at(modName);
	for (auto sett : this->settings) {
		switch (sett->valueType) {
		case ValueType::FLOAT_T:
			obj.emplace(sett->name, sett->value->_float);
			break;
		case ValueType::DOUBLE_T:
			obj.emplace(sett->name, sett->value->_double);
			break;
		case ValueType::INT64_T:
			obj.emplace(sett->name, sett->value->int64);
			break;
		case ValueType::ENUM_T:
		case ValueType::KEYBIND_T:
		case ValueType::INT_T:
			obj.emplace(sett->name, sett->value->_int);
			break;
		case ValueType::BOOL_T:
			obj.emplace(sett->name, sett->value->_bool);
			break;
		case ValueType::TEXT_T:
			obj.emplace(sett->name, *sett->value->text);
			break;
		case ValueType::SETTING_GROUP_T: {
			json obj2 = {};
			sett->groups[0]->onSaveConfig(&obj2);
			obj.emplace(sett->name, obj2);
		} break;
		case ValueType::BOOL_SETTING_GROUP_T: {
			json obj2 = {};
			obj2.emplace("value", sett->value->_bool);
			json groupObj = {};
			sett->groups[0]->onSaveConfig(&groupObj);
			obj2.emplace("group", groupObj);
			obj.emplace(sett->name, obj2);
		} break;
		case ValueType::ENUM_SETTING_GROUP_T: {
			json obj2 = {};
			obj2.emplace("value", sett->value->_int);
			json groups;

			int num = 0;
			for (auto it : sett->groups) {
				json obj3 = {};
				if (it == nullptr) {
					groups.emplace(std::to_string(num), obj3);
					num++;
					continue;
				}
				else {
					it->onSaveConfig(&obj3);
					groups.emplace(std::to_string(num), obj3);
				}
				num++;
			}

			obj2.emplace("groups", groups);
			obj.emplace(sett->name, obj2);
		} break;
		}
	}

	conf->emplace(modName.c_str(), obj);
}

void SettingGroup::onSaveConfig(void* confVoid) {
	json* obj = reinterpret_cast<json*>(confVoid);
	const char* name = parent->name;
	//auto obj = conf->at(modName);

	for (auto sett : entries) {
		switch (sett->valueType) {
		case ValueType::FLOAT_T:
			obj->emplace(sett->name, sett->value->_float);
			break;
		case ValueType::DOUBLE_T:
			obj->emplace(sett->name, sett->value->_double);
			break;
		case ValueType::INT64_T:
			obj->emplace(sett->name, sett->value->int64);
			break;
		case ValueType::ENUM_T:
		case ValueType::KEYBIND_T:
		case ValueType::INT_T:
			obj->emplace(sett->name, sett->value->_int);
			break;
		case ValueType::BOOL_T:
			obj->emplace(sett->name, sett->value->_bool);
			break;
		case ValueType::TEXT_T:
			obj->emplace(sett->name, *sett->value->text);
			break;
		case ValueType::SETTING_GROUP_T: {
			json obj2 = {};
			sett->groups[0]->onSaveConfig(&obj2);
			obj->emplace(sett->name, obj2);
		} break;
		case ValueType::BOOL_SETTING_GROUP_T: {
			json obj2 = {};
			obj2.emplace("value", sett->value->_bool);
			json groupObj = {};
			sett->groups[0]->onSaveConfig(&groupObj);
			obj2.emplace("group", groupObj);
			obj->emplace(sett->name, obj2);
		} break;
		case ValueType::ENUM_SETTING_GROUP_T: {
			json obj2 = {};
			obj2.emplace("value", sett->value->_int);
			json groups;

			int num = 0;

			for (auto it : sett->groups) {
				json obj3 = {};
				if (it == nullptr) {
					groups.emplace(std::to_string(num), obj3);
					num++;
					continue;
				}
				else {
					it->onSaveConfig(&obj3);
					groups.emplace(std::to_string(num), obj3);
				}
				num++;
			}

			obj2.emplace("groups", groups);
			obj->emplace(sett->name, obj2);
		} break;
		}
	}
}

#pragma warning(pop)



bool IModule::isHoldMode() {
	return false;
}

void IModule::setEnabled(bool enabled) {
	if (this->enabled != enabled) {
		this->enabled = enabled;

		// Toggle Notifications
		static auto configMenu = moduleMgr->getModule<ConfigManagerMod>();
		static auto notifications = moduleMgr->getModule<Notifications>();
		static auto interfaceMod = moduleMgr->getModule<Interface>();
		static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
		static auto antiBotMod = moduleMgr->getModule<AntiBot>();
		static auto autoPlay = moduleMgr->getModule<AutoPlay>();
		static auto path = moduleMgr->getModule<FollowPath>();
		bool shouldShow = true;

		string screenName(g_Hooks.currentScreenName);
		auto player = g_Data.getLocalPlayer();
		if (player == nullptr ||
			clickGUI->isEnabled() || configMenu->isEnabled() ||
			path->isEnabled() || autoPlay->isEnabled() || isHoldMode() || strcmp(screenName.c_str(), "start_screen") == 0)
			shouldShow = false;

		if (notifications->showToggle && shouldShow && !((GameData::isKeyDown('L') && GameData::isKeyDown(VK_CONTROL)) || GameData::shouldTerminate())) {
			if (notifications->isEnabled()) {
				auto state = enabled ? " Enabled" : " Disabled";
				string noftitle = "Notification:";
				if (notifications->mode.getSelectedValue() == 1) noftitle = "Toggled:";
				auto notification = g_Data.addNotification(noftitle, string(string(state) + " " + this->getRawModuleName())); notification->duration = 3.f; if (notifications->color) {
					if (enabled) { notification->colorR = 42; notification->colorG = 113; notification->colorB = 47; }
					else { notification->colorR = 127; notification->colorG = 34; notification->colorB = 25; }
				}
			}
		}

		if (enabled) {
			this->onEnable();
			static auto toggleSounds = moduleMgr->getModule<ToggleSounds>();
			if (toggleSounds->isEnabled() && g_Data.getLocalPlayer() != nullptr && !((GameData::isKeyDown('L') && GameData::isKeyDown(VK_CONTROL)) || GameData::shouldTerminate())) {
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;

				switch (toggleSounds->sound.getSelectedValue()) {
				case 0: // Click
					level->playSound("random.click", *player->getPos(), toggleSounds->volume, 0.6); // 0.6
					break;
				case 1: // Piston
					level->playSound("tile.piston.out", *player->getPos(), toggleSounds->volume, 1);
					break;
				case 2: // Sigma
					Utils::systemPlay("Sigma_Enable.wav");
					break;
				case 3: // Bell
					level->playSound("note.bell", *player->getPos(), toggleSounds->volume, 0.6);
					break;
				}
			}
		}
		else {
			this->onDisable();
			static auto toggleSounds = moduleMgr->getModule<ToggleSounds>();
			if (toggleSounds->isEnabled() && g_Data.getLocalPlayer() != nullptr && !((GameData::isKeyDown('L') && GameData::isKeyDown(VK_CONTROL)) || GameData::shouldTerminate())) {
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;

				switch (toggleSounds->sound.getSelectedValue()) {
				case 0: // Click
					level->playSound("random.click", *player->getPos(), toggleSounds->volume, 0.5); // 0.6
					break;
				case 1: // Piston
					level->playSound("tile.piston.out", *player->getPos(), toggleSounds->volume, 1);
					break;
				case 2: // Sigma
					Utils::systemPlay("Sigma_Disable.wav");
					break;
				case 3: // Bell
					level->playSound("note.bell", *player->getPos(), toggleSounds->volume, 0.5);
					break;
				}
			}
		}
	}
}

void IModule::toggle() {
	setEnabled(!this->enabled);
}

bool IModule::isEnabled() {
	return this->enabled;
}

const char* IModule::getTooltip() {
	return this->tooltip;
}
void IModule::onAttack(C_Entity*) {
}
bool IModule::callWhenDisabled() {
	return false;
}
void IModule::onMove(C_MoveInputHandler*) {
}
void IModule::onLevelRender() {
}
void IModule::clientMessageF(const char* fmt, ...) {
	va_list arg;
	va_start(arg, fmt);

	char message[300];
	vsprintf_s(message, 300, fmt, arg);

	GameData::log("%s", message);

	va_end(arg);
}

void SettingEntry::makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt() {
	switch (valueType) {
	case ValueType::ENUM_SETTING_GROUP_T:
	case ValueType::ENUM_T:
		value->_int = std::max(0, std::min(reinterpret_cast<SettingEnum*>(extraData)->GetCount() - 1, value->_int));
		break;
	case ValueType::BOOL_SETTING_GROUP_T:
	case ValueType::BOOL_T:
		break;
	case ValueType::INT64_T:
		value->int64 = std::max(minValue->int64, std::min(maxValue->int64, value->int64));
		break;
	case ValueType::DOUBLE_T:
		value->_double = std::max(minValue->_double, std::min(maxValue->_double, value->_double));
		break;
	case ValueType::FLOAT_T:
		value->_float = std::max(minValue->_float, std::min(maxValue->_float, value->_float));
		break;
	case ValueType::KEYBIND_T:
	case ValueType::INT_T:
		value->_int = std::max(minValue->_int, std::min(maxValue->_int, value->_int));
		break;
	case ValueType::TEXT_T:
	case ValueType::SETTING_GROUP_T:
		break;
	default:
		logF("unrecognized value %i", valueType);
		break;
	}

	if (!groups.empty()) {
		for (auto it : groups) {
			if (it != nullptr) {
				for (auto i : it->entries)
					i->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
			}
		}
	}
}

SettingEntry* SettingEntry::addSettingGroup(int _enum, SettingGroup* group) {
	int enumReal = -1;

	SettingEnum* dat = (SettingEnum*)extraData;

	int i = 0;
	for (auto it = dat->Entrys.begin(); it < dat->Entrys.end(); it++, i++) {
		if (it->GetValue() == _enum)
			enumReal = i;
	}

	if (enumReal < 0 || enumReal >= groups.size())
		return this;

	groups[enumReal] = group;

	group->parent = this;

	return this;
}

SettingEntry* SettingEntry::addSettingGroup(SettingGroup* group) {
	groups[0] = group;

	group->parent = this;

	return this;
}
