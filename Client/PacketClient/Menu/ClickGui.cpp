 #include "ClickGui.h"

#include <Windows.h>
#include "../../Utils/Json.hpp"
#include "../../Utils/Logger.h"

using namespace std;
bool isLeftClickDown = false;
bool isRightClickDown = false;
bool shouldToggleLeftClick = false;  // If true, toggle the focused module
bool shouldToggleRightClick = false;
bool resetStartPos = true;
bool initialised = false;
int scrollingDirection = 0;

struct SavedWindowSettings {
	vec2_t pos = { -1, -1 };
	bool isExtended = true;
	const char* name = "";
};

SettingEntry* keybindMenuCurrent = nullptr;  // What setting is currently capturing the user's input?
int newKeybind = 0;
bool isCapturingKey = false;
bool shouldStopCapturing = false;

bool isConfirmingKey = false;  // Show a cancel and save button

map<unsigned int, shared_ptr<ClickWindow>> windowMap;
map<unsigned int, shared_ptr<ClickWindow2>> windowMap2;
map<unsigned int, SavedWindowSettings> savedWindowSettings;

bool isDragging = false;
unsigned int draggedWindow = -1;
vec2_t dragStart = vec2_t();

unsigned int focusedElement = -1;
bool isFocused = false;

//static constexpr float textPadding = 2.0f;
static constexpr float textPadding = 1.0f;
static constexpr float textSize = 1.0f;
static constexpr float textHeight = textSize * 9.5f;
static constexpr float categoryMargin = 0.5f;
static constexpr float paddingRight = 13.5f;
static constexpr float crossSize = textHeight / 2.f;
static constexpr float crossWidth = 0.2f;

static float rcolors[4];  // Rainbow color array RGBA

float currentYOffset = 0;
float currentXOffset = 0;

int timesRendered = 0;

void ClickGui::getModuleListByCategory(Category category, vector<shared_ptr<IModule>>* modList) {
	auto lock = moduleMgr->lockModuleList();
	vector<shared_ptr<IModule>>* moduleList = moduleMgr->getModuleList();

	for (auto& it : *moduleList) {
		if (it->getCategory() == category) modList->push_back(it);
	}
}

std::shared_ptr<ClickWindow> ClickGui::getWindow(const char* name) {
	unsigned int id = Utils::getCrcHash(name);

	auto search = windowMap.find(id);
	if (search != windowMap.end()) {  // Window exists already
		return search->second;
	}
	else {  // Create window
	 // TODO: restore settings for position etc
		std::shared_ptr<ClickWindow> newWindow = std::make_shared<ClickWindow>();
		newWindow->name = name;

		auto savedSearch = savedWindowSettings.find(id);
		if (savedSearch != savedWindowSettings.end()) {  // Use values from config
			newWindow->isExtended = savedSearch->second.isExtended;
			if (savedSearch->second.pos.x > 0)
				newWindow->pos = savedSearch->second.pos;
		}

		windowMap.insert(std::make_pair(id, newWindow));
		return newWindow;
	}
}

std::shared_ptr<ClickModule> ClickGui::getClickModule(std::shared_ptr<ClickWindow> window, const char* name) {
	unsigned int id = Utils::getCrcHash(name);

	auto search = window->moduleMap.find(id);
	if (search != window->moduleMap.end()) {  // Window exists already
		return search->second;
	}
	else {  // Create window
	 // TODO: restore settings for position etc
		std::shared_ptr<ClickModule> newModule = std::make_shared<ClickModule>();

		window->moduleMap.insert(std::make_pair(id, newModule));
		return newModule;
	}
}

shared_ptr<ClickModule2> ClickGui::getClickModule2(shared_ptr<ClickWindow2> window, const char* name) {
	unsigned int id = Utils::getCrcHash(name);

	auto search = window->moduleMap2.find(id);
	if (search != window->moduleMap2.end()) {  // Window exists already
		return search->second;
	}
	else {  // Create window
	 // TODO: restore settings for position etc
		shared_ptr<ClickModule2> newModule = make_shared<ClickModule2>();

		window->moduleMap2.insert(make_pair(id, newModule));
		return newModule;
	}
}
shared_ptr<ClickWindow2> ClickGui::getWindow2(const char* name) {
	unsigned int id = Utils::getCrcHash(name);

	auto search = windowMap2.find(id);
	if (search != windowMap2.end()) {  // Window exists already
		return search->second;
	}
	else {  // Create window
	 // TODO: restore settings for position etc
		shared_ptr<ClickWindow2> newWindow = make_shared<ClickWindow2>();
		newWindow->name = name;

		auto savedSearch = savedWindowSettings.find(id);
		if (savedSearch != savedWindowSettings.end()) {  // Use values from config
			newWindow->isExtended = savedSearch->second.isExtended;
			if (savedSearch->second.pos.x > 0) newWindow->pos = savedSearch->second.pos;
		}

		windowMap2.insert(make_pair(id, newWindow));
		return newWindow;
	}
}

void ClickGui::renderLabel(const char* text) { }

void ClickGui::renderTooltip(string* text) {
	vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
	float lFPS = DrawUtils::getTextWidth(text, 1) + 6.5;
	float posY = windowSize.y - 24;
	vec4_t rectPos = vec4_t(2, posY + 4, lFPS + 1, posY + 20);
	vec2_t textPos = vec2_t(rectPos.x + textPadding + 2, rectPos.y + 4);
	DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, 20));
	DrawUtils::drawRectangle(rectPos, MC_Color(0, 0, 0), 0.6f, 0.5f);
	DrawUtils::drawText(textPos, text, MC_Color(255, 255, 255), 1);
}
#pragma region PacketOld
void ClickGui::renderPacketOldCategory(Category category, MC_Color categoryColor) {
	static constexpr float textHeight = textSize * 9.f;
	const char* categoryName = ClickGui::catToName(category);
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const std::shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;

		}
	}
	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset;

	// Get All Modules in our category
	std::vector<std::shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);

	// Get max width of all text
	{
		for (auto& it : ModuleList) {
			std::string label = "-------------";
			windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&label, textSize, Fonts::SMOOTH));
		}
	}

	vector<shared_ptr<IModule>> moduleList;
	{
		vector<int> toIgniore;
		int moduleCount = (int)ModuleList.size();
		for (int i = 0; i < moduleCount; i++) {
			float bestWidth = 1.f;
			int bestIndex = 1;

			for (int j = 0; j < ModuleList.size(); j++) {
				bool stop = false;
				for (int bruhwth = 0; bruhwth < toIgniore.size(); bruhwth++)
					if (j == toIgniore[bruhwth]) {
						stop = true;
						break;
					}
				if (stop)
					continue;

				string t = ModuleList[j]->getRawModuleName();
				float textWidthRn = DrawUtils::getTextWidth(&t, textSize, Fonts::SMOOTH);
				if (textWidthRn > bestWidth) {
					bestWidth = textWidthRn;
					bestIndex = j;
				}
			}
			moduleList.push_back(ModuleList[bestIndex]);
			toIgniore.push_back(bestIndex);
		}
	}

	const float xEnd = currentXOffset + windowSize->x + paddingRight;

	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f) ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 0.75f) + 0.5f /*fix flickering related to rounding errors*/;
		int moduleIndex = 0;
		for (auto& mod : moduleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;
			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			if (ourWindow->isInAnimation) { // Estimate, we don't know about module settings yet
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			std::string textStr = mod->getRawModuleName();

			vec2_t textPos = vec2_t(currentXOffset + textPadding + 45, currentYOffset + textPadding + 5);
			vec4_t rectPos = vec4_t(
				currentXOffset, currentYOffset, xEnd,
				currentYOffset + textHeight + (textPadding * 2));

			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			// Background
			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					DrawUtils::fillRectangle(rectPos, MC_Color(25, 25, 25), 0.5);
					std::string tooltip = mod->getTooltip();
					static auto clickGuiMod = moduleMgr->getModule<ClickGUIMod>();
					if (clickGuiMod->showTooltips && !tooltip.empty()) renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
				else {
					DrawUtils::fillRectangleA(rectPos, mod->isEnabled() ? MC_Color(32, 32, 32, clickGUI->opacity) : MC_Color(8, 8, 8, clickGUI->opacity));
				}
			}

			// Text
			if (allowRender) DrawUtils::drawCenteredString(textPos, &textStr, textSize, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);
			//if (allowRender) DrawUtils::drawText(textPos, &textStr, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);

			// Settings
			{
				std::vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
					}
					GuiUtils::drawCrossLine(vec2_t(
						currentXOffset + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
						currentYOffset + textPadding + (textHeight / 2)),
						MC_Color(255, 255, 255), crossWidth, crossSize, !clickMod->isExtended);

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset + textPadding + 5, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							string len = "saturation  ";
							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									vec4_t boxPos = vec4_t(
										rectPos.z + textPadding - 15,
										rectPos.y + textPadding + 2,
										rectPos.z + textHeight - textPadding - 15,
										rectPos.y + textHeight - textPadding + 2);

									DrawUtils::drawRectangle(boxPos, MC_Color(255, 255, 255), isFocused ? 1 : 0.8f, 0.5f);

									if (setting->value->_bool) {
										DrawUtils::setColor(28, 107, 201, 1);
										boxPos.x += textPadding;
										boxPos.y += textPadding;
										boxPos.z -= textPadding;
										boxPos.w -= textPadding;
										DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
										DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
									}
								}

								// Text
								{
									// Convert first letter to uppercase for more friendlieness
									char name[0x21]; sprintf_s(name, 0x21, "%s", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									string elTexto = name;

									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140), true, true, true);
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								// Text and background
								float settingStart = currentYOffset;
								{
									char name[0x22]; sprintf_s(name, +"%s:", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();

									char name2[0x21]; sprintf_s(name2, 0x21, " %s", i.GetName().c_str()); if (name2[0] != 0) name2[0] = toupper(name2[0]);
									string elTexto2 = name2;

									string elTexto = string(GRAY) + name + string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight - 0.5 + (textPadding * 2);
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true); //enum aids
									GuiUtils::drawCrossLine(vec2_t(
										currentXOffset + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
										currentYOffset + textPadding + (textHeight / 2)),
										MC_Color(255, 255, 255), crossWidth, crossSize, !setting->minValue->_bool);
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char str[10]; sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5, currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5, currentYOffset - textPadding + textHeight + 3.f);
									// Visuals & Logic
									{
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										rect.z = rect.z - 10;
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);
										if (value > maxValue) value = maxValue; value /= maxValue;
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;

										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									// Convert first letter to uppercase for more friendlieness
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10]; sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 15);
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5,
										currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5,
										currentYOffset - textPadding + textHeight + 3.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background

										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);

										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight + (textPadding * 2);
			}
		}

		vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

		if (winRectPos.contains(&mousePos)) {
			if (scrollingDirection > 0 && overflowing) ourWindow->yOffset += scrollingDirection;
			else if (scrollingDirection < 0) ourWindow->yOffset += scrollingDirection;
			scrollingDirection = 0;
			if (ourWindow->yOffset < 0) ourWindow->yOffset = 0;
		}
	}
	DrawUtils::flush();
	// Draw Category Header
	{
		vec2_t textPos = vec2_t(currentXOffset + textPadding, categoryHeaderYOffset + textPadding);
		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin, categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin,
			categoryHeaderYOffset + textHeight + (textPadding * 2));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : moduleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && ourWindow->animation == 0) ourWindow->animation = 0.2f;
				else if (!ourWindow->isExtended && ourWindow->animation == 1) ourWindow->animation = 0;
				ourWindow->isInAnimation = true;

				for (auto& mod : moduleList) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		// Draw category header
		{
			// Draw Text
			string textStr = categoryName;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255), textSize);
			DrawUtils::fillRectangle(rectPos, MC_Color(0, 0, 0), 0.5f);
			// Draw Dash
			GuiUtils::drawCrossLine(vec2_t(
				currentXOffset + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
				categoryHeaderYOffset + textPadding + (textHeight / 2)),
				MC_Color(255, 255, 255), crossWidth, crossSize, !ourWindow->isExtended);
			//DrawUtils::drawRoundRectangle(rectTest, MC_Color(255, 255, 255), true);
			//DrawUtils::drawRoundRectangle(rectTest2, MC_Color(100, 100, 100), true);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	moduleList.clear();
	DrawUtils::flush();
}
#pragma endregion
#pragma region Packet
void ClickGui::renderPacketCategory(Category category, MC_Color categoryColor) {
	static constexpr float textHeight = textSize * 9.f;
	const char* categoryName = ClickGui::catToName(category);
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const std::shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;

		}
	}
	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset;

	// Get All Modules in our category
	std::vector<std::shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);

	// Get max width of all text
	{
		for (auto& it : ModuleList) {
			std::string label = "-------------";
			windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&label, textSize, Fonts::SMOOTH));
		}
	}

	vector<shared_ptr<IModule>> moduleList;
	{
		vector<int> toIgniore;
		int moduleCount = (int)ModuleList.size();
		for (int i = 0; i < moduleCount; i++) {
			float bestWidth = 1.f;
			int bestIndex = 1;

			for (int j = 0; j < ModuleList.size(); j++) {
				bool stop = false;
				for (int bruhwth = 0; bruhwth < toIgniore.size(); bruhwth++)
					if (j == toIgniore[bruhwth]) {
						stop = true;
						break;
					}
				if (stop)
					continue;

				string t = ModuleList[j]->getRawModuleName();
				float textWidthRn = DrawUtils::getTextWidth(&t, textSize, Fonts::SMOOTH);
				if (textWidthRn > bestWidth) {
					bestWidth = textWidthRn;
					bestIndex = j;
				}
			}
			moduleList.push_back(ModuleList[bestIndex]);
			toIgniore.push_back(bestIndex);
		}
	}

	const float xEnd = currentXOffset + windowSize->x + paddingRight;

	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f) ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 0.75f) + 0.5f /*fix flickering related to rounding errors*/;
		int moduleIndex = 0;
		for (auto& mod : moduleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;
			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			if (ourWindow->isInAnimation) { // Estimate, we don't know about module settings yet
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			std::string textStr = mod->getRawModuleName();

			vec2_t textPos = vec2_t(currentXOffset + textPadding + 45, currentYOffset + textPadding + 5);
			vec4_t rectPos = vec4_t(
				currentXOffset, currentYOffset, xEnd,
				currentYOffset + textHeight + (textPadding * 2));

			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			// Background
			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					DrawUtils::fillRectangle(rectPos, MC_Color(25, 25, 25), 0.5);
					std::string tooltip = mod->getTooltip();
					static auto clickGuiMod = moduleMgr->getModule<ClickGUIMod>();
					if (clickGuiMod->showTooltips && !tooltip.empty()) renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
				else {
					DrawUtils::fillRectangleA(rectPos, mod->isEnabled() ? MC_Color(32, 32, 32, clickGUI->opacity) : MC_Color(8, 8, 8, clickGUI->opacity));
				}
			}

			// Text
			if (allowRender) DrawUtils::drawCenteredString(textPos, &textStr, textSize, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);
			//if (allowRender) DrawUtils::drawText(textPos, &textStr, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);

			// Settings
			{
				std::vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
					}

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset + textPadding + 5, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							string len = "saturation  ";
							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									vec4_t boxPos = vec4_t(
										rectPos.z + textPadding - 15,
										rectPos.y + textPadding + 2,
										rectPos.z + textHeight - textPadding - 15,
										rectPos.y + textHeight - textPadding + 2);

									DrawUtils::drawRectangle(boxPos, MC_Color(255, 255, 255), isFocused ? 1 : 0.8f, 0.5f);

									if (setting->value->_bool) {
										DrawUtils::setColor(28, 107, 201, 1);
										boxPos.x += textPadding;
										boxPos.y += textPadding;
										boxPos.z -= textPadding;
										boxPos.w -= textPadding;
										DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
										DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
									}
								}

								// Text
								{
									// Convert first letter to uppercase for more friendlieness
									char name[0x21]; sprintf_s(name, 0x21, "%s", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									string elTexto = name;

									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140), true, true, true);
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								// Text and background
								float settingStart = currentYOffset;
								{
									char name[0x22]; sprintf_s(name, +"%s:", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();

									char name2[0x21]; sprintf_s(name2, 0x21, " %s", i.GetName().c_str()); if (name2[0] != 0) name2[0] = toupper(name2[0]);
									string elTexto2 = name2;

									string elTexto = string(GRAY) + name + string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight - 0.5 + (textPadding * 2);
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true); //enum aids
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char str[10]; sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5, currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5, currentYOffset - textPadding + textHeight + 3.f);
									// Visuals & Logic
									{
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										rect.z = rect.z - 10;
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);
										if (value > maxValue) value = maxValue; value /= maxValue;
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;

										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									// Convert first letter to uppercase for more friendlieness
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10]; sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 15);
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5,
										currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5,
										currentYOffset - textPadding + textHeight + 3.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background

										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);

										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight + (textPadding * 2);
			}
		}

		vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

		if (winRectPos.contains(&mousePos)) {
			if (scrollingDirection > 0 && overflowing) ourWindow->yOffset += scrollingDirection;
			else if (scrollingDirection < 0) ourWindow->yOffset += scrollingDirection;
			scrollingDirection = 0;
			if (ourWindow->yOffset < 0) ourWindow->yOffset = 0;
		}
	}
	DrawUtils::flush();
	// Draw Category Header
	{
		vec2_t textPos = vec2_t(currentXOffset + textPadding, categoryHeaderYOffset + textPadding);
		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin, categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin,
			categoryHeaderYOffset + textHeight + (textPadding * 2));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : moduleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && ourWindow->animation == 0) ourWindow->animation = 0.2f;
				else if (!ourWindow->isExtended && ourWindow->animation == 1) ourWindow->animation = 0;
				ourWindow->isInAnimation = true;

				for (auto& mod : moduleList) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		//Draw a bit more then just the HudEditor button
		/*{
			std::vector<SettingEntry*>* settings = clickGUI->getSettings();
			string textStr = "Packet";
			float textStrLen = DrawUtils::getTextWidth(&string("------------")) - 2.f;
			float textStrLen2 = DrawUtils::getTextWidth(&string("--------------"));
			float stringLen = DrawUtils::getTextWidth(&textStr) + 2;
			vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
			float mid = windowSize2.x / 2 - 20;

			vec4_t rect = vec4_t(mid, 0, mid + textStrLen, 18);
			vec4_t settingsRect = vec4_t(rect.x + stringLen + 3, rect.y + 2, rect.x + stringLen + 17, rect.y + 16);
			vec4_t hudEditor = vec4_t(rect.x + stringLen + 19, rect.y + 2, rect.x + stringLen + 33, rect.y + 16);

			if (hudEditor.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);

			DrawUtils::fillRectangleA(rect, MC_Color(37, 39, 43, 255));
			DrawUtils::fillRectangleA(settingsRect, MC_Color(9, 12, 16, 255));
			DrawUtils::fillRectangleA(hudEditor, MC_Color(15, 20, 26, 255));
			DrawUtils::drawText(vec2_t(rect.x + 3, rect.y + 4), &textStr, MC_Color(255, 255, 255), 1.f, 1.f, true);

			float ourOffset = 17;
			static bool extended = false;

			if (settingsRect.contains(&mousePos) && shouldToggleRightClick) {
				shouldToggleRightClick = false;
				extended = !extended;
			}

			vec4_t idkRect = vec4_t(settingsRect.x, ourOffset, settingsRect.x + textStrLen2, ourOffset + 16);
			for (int t = 0; t < 4; t++)	idkRect.w += ourOffset;

			if (extended) {
				DrawUtils::fillRectangleA(idkRect, MC_Color(45, 45, 45, 255));
				string stringAids;
				string stringAids2;
				if (clickGUI->theme.getSelectedValue() == 0) stringAids = "Theme: Packet";
				if (clickGUI->theme.getSelectedValue() == 1) stringAids = "Theme: Vape";
				if (clickGUI->theme.getSelectedValue() == 2) stringAids = "Theme: Astolfo";

				if (clickGUI->color.getSelectedValue() == 0) stringAids2 = "Color: Rainbow";
				if (clickGUI->color.getSelectedValue() == 1) stringAids2 = "Color: Astolfo";
				if (clickGUI->color.getSelectedValue() == 2) stringAids2 = "Color: Wave";
				if (clickGUI->color.getSelectedValue() == 3) stringAids2 = "Color: RainbowWave";

				vec4_t selectableSurface = vec4_t(settingsRect.x, ourOffset + 2.f, settingsRect.x + textStrLen2, ourOffset + 17.f);
				vec4_t selectableSurface2 = vec4_t(settingsRect.x, ourOffset + 17.f, settingsRect.x + textStrLen2, ourOffset + 37.f);
				DrawUtils::drawText(vec2_t(selectableSurface.x + 2, selectableSurface.y + 3), &stringAids, MC_Color(255, 255, 255), 1.f, 1.f, true);
				DrawUtils::drawText(vec2_t(selectableSurface2.x + 2, selectableSurface2.y + 3), &stringAids2, MC_Color(255, 255, 255), 1.f, 1.f, true);

				if (selectableSurface.contains(&mousePos) && shouldToggleLeftClick) {
					clickGUI->theme.SelectNextValue(false);
					shouldToggleLeftClick = false;
				}
				if (selectableSurface.contains(&mousePos) && shouldToggleRightClick) {
					clickGUI->theme.SelectNextValue(true);
					shouldToggleLeftClick = false;
				}
				if (selectableSurface2.contains(&mousePos) && shouldToggleLeftClick) {
					clickGUI->color.SelectNextValue(false);
					shouldToggleLeftClick = false;
				}
				if (selectableSurface2.contains(&mousePos) && shouldToggleRightClick) {
					clickGUI->color.SelectNextValue(true);
					shouldToggleLeftClick = false;
				}
			}
		}*/

		// Draw category header
		{
			// Draw Text
			string textStr = categoryName;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255), textSize);
			DrawUtils::fillRectangle(rectPos, MC_Color(0, 0, 0), 0.5f);
			DrawUtils::drawRoundRectangle(rectTest, MC_Color(255, 255, 255), true);
			//DrawUtils::drawRoundRectangle(rectTest2, MC_Color(100, 100, 100), true);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	moduleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region TANA
void ClickGui::renderTANACategory(Category category, MC_Color categoryColor) {
	static constexpr float textHeight = textSize * 9.f;
	const char* categoryName = ClickGui::catToName(category);
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const std::shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	float yot = g_Data.getGuiData()->windowSize.x;
	float yot2 = g_Data.getGuiData()->windowSize.y;
	ourWindow->pos.y = 4;
	switch (category) {
	case Category::COMBAT:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 7.5f;
		break;
	case Category::VISUAL:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 4.1f;
		break;
	case Category::MOVEMENT:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 5.6f * 2.f;
		break;
	case Category::PLAYER:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 4.25f * 2.f;
		break;
	case Category::EXPLOIT:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 3.4f * 2.f;
		break;
	case Category::OTHER:
		ourWindow->pos.x = yot / 7.5f;
		ourWindow->pos.y = yot2 / 2.9f * 2.05f;
		break;
	case Category::UNUSED:
		ourWindow->pos.x = yot / 1.6f * 2.2f;
		break;
	case Category::CUSTOM:
		ourWindow->pos.x = yot / 5.6f * 2.f;
		break;

	}
	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset;
	float currentXOffset2 = currentXOffset + 20; //itwas
	currentXOffset2 = yot / 4.25f * 2.f;
	float currentYOffset2 = yot2 / 7.5f; //itwas

	// Get All Modules in our category
	std::vector<std::shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);

	// Get max width of all text
	{
		for (auto& it : ModuleList) {
			std::string label = "-------------";
			windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&label, textSize, Fonts::SMOOTH));
		}
	}

	vector<shared_ptr<IModule>> moduleList;
	{
		vector<int> toIgniore;
		int moduleCount = (int)ModuleList.size();
		for (int i = 0; i < moduleCount; i++) {
			float bestWidth = 1.f;
			int bestIndex = 1;

			for (int j = 0; j < ModuleList.size(); j++) {
				bool stop = false;
				for (int bruhwth = 0; bruhwth < toIgniore.size(); bruhwth++)
					if (j == toIgniore[bruhwth]) {
						stop = true;
						break;
					}
				if (stop)
					continue;

				string t = ModuleList[j]->getRawModuleName();
				float textWidthRn = DrawUtils::getTextWidth(&t, textSize, Fonts::SMOOTH);
				if (textWidthRn > bestWidth) {
					bestWidth = textWidthRn;
					bestIndex = j;
				}
			}
			moduleList.push_back(ModuleList[bestIndex]);
			toIgniore.push_back(bestIndex);
		}
	}

	const float xEnd = currentXOffset2 + windowSize->x + paddingRight;

	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f) ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 0.75f) + 0.5f /*fix flickering related to rounding errors*/;
		int moduleIndex = 0;
		for (auto& mod : moduleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;
			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			if (ourWindow->isInAnimation) { // Estimate, we don't know about module settings yet
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			std::string textStr = mod->getRawModuleName();

			vec2_t textPos = vec2_t(currentXOffset2 + textPadding + 45, currentYOffset2 + textPadding + 5);
			vec4_t rectPos = vec4_t(
				currentXOffset2, currentYOffset2, xEnd,
				currentYOffset2 + textHeight + (textPadding * 2));

			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			// Background
			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					DrawUtils::fillRectangle(rectPos, MC_Color(25, 25, 25), 0.5);
					std::string tooltip = mod->getTooltip();
					static auto clickGuiMod = moduleMgr->getModule<ClickGUIMod>();
					if (clickGuiMod->showTooltips && !tooltip.empty()) renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
				else {
					DrawUtils::fillRectangleA(rectPos, mod->isEnabled() ? MC_Color(32, 32, 32, clickGUI->opacity) : MC_Color(8, 8, 8, clickGUI->opacity));
				}
			}

			// Text
			if (allowRender) DrawUtils::drawCenteredString(textPos, &textStr, textSize, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);
			//if (allowRender) DrawUtils::drawText(textPos, &textStr, mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(100, 100, 100), true);

			// Settings
			{
				std::vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
					}
					GuiUtils::drawCrossLine(vec2_t(
						currentXOffset2 + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
						currentYOffset + textPadding + (textHeight / 2)),
						MC_Color(255, 255, 255), crossWidth, crossSize, !clickMod->isExtended);

					currentYOffset2 += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset2 + textPadding + 5, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset2/*itwas*/, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							string len = "saturation  ";
							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									vec4_t boxPos = vec4_t(
										rectPos.z + textPadding - 15,
										rectPos.y + textPadding + 2,
										rectPos.z + textHeight - textPadding - 15,
										rectPos.y + textHeight - textPadding + 2);

									DrawUtils::drawRectangle(boxPos, MC_Color(255, 255, 255), isFocused ? 1 : 0.8f, 0.5f);

									if (setting->value->_bool) {
										DrawUtils::setColor(28, 107, 201, 1);
										boxPos.x += textPadding;
										boxPos.y += textPadding;
										boxPos.z -= textPadding;
										boxPos.w -= textPadding;
										DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
										DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
									}
								}

								// Text
								{
									// Convert first letter to uppercase for more friendlieness
									char name[0x21]; sprintf_s(name, 0x21, "%s", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									string elTexto = name;

									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140), true, true, true);
									currentYOffset2 += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								// Text and background
								float settingStart = currentYOffset;
								{
									char name[0x22]; sprintf_s(name, +"%s:", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();

									char name2[0x21]; sprintf_s(name2, 0x21, " %s", i.GetName().c_str()); if (name2[0] != 0) name2[0] = toupper(name2[0]);
									string elTexto2 = name2;

									string elTexto = string(GRAY) + name + string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight - 0.5 + (textPadding * 2);
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true); //enum aids
									GuiUtils::drawCrossLine(vec2_t(
										currentXOffset2 + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
										currentYOffset + textPadding + (textHeight / 2)),
										MC_Color(255, 255, 255), crossWidth, crossSize, !setting->minValue->_bool);
									currentYOffset2 += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset2 += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char str[10]; sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset2 += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset2 + textPadding + 5, currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5, currentYOffset - textPadding + textHeight + 3.f);
									// Visuals & Logic
									{
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										rect.z = rect.z - 10;
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);
										if (value > maxValue) value = maxValue; value /= maxValue;
										const float endXlol = (xEnd - textPadding) - (currentXOffset2 + textPadding + 10);
										value *= endXlol;

										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										/* {
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}*/

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset2 += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									// Convert first letter to uppercase for more friendlieness
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10]; sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									string elTexto = name + text;
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 15);
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset2 += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset2 + textPadding + 5,
										currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5,
										currentYOffset - textPadding + textHeight + 3.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background

										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset2 + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);

										}

										// Drag Logic
										/* {
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}*/

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset2 += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset2 += textHeight + (textPadding * 2);
			}
		}

		vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

		if (winRectPos.contains(&mousePos)) {
			if (scrollingDirection > 0 && overflowing) ourWindow->yOffset += scrollingDirection;
			else if (scrollingDirection < 0) ourWindow->yOffset += scrollingDirection;
			scrollingDirection = 0;
			if (ourWindow->yOffset < 0) ourWindow->yOffset = 0;
		}
	}
	DrawUtils::flush();
	// Draw Category Header
	{
		vec2_t textPos = vec2_t(currentXOffset + textPadding, categoryHeaderYOffset + textPadding);
		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin, categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin,
			categoryHeaderYOffset + textHeight + (textPadding * 2));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : moduleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && ourWindow->animation == 0) ourWindow->animation = 0.2f;
				else if (!ourWindow->isExtended && ourWindow->animation == 1) ourWindow->animation = 0;
				ourWindow->isInAnimation = true;

				for (auto& mod : moduleList) {
					std::shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		/* {
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}*/

		// Draw category header
		{
			// Draw Text
			string textStr = categoryName;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255), textSize);
			DrawUtils::fillRectangle(rectPos, MC_Color(0, 0, 0), 0.5f);
			// Draw Dash
			GuiUtils::drawCrossLine(vec2_t(
				currentXOffset + windowSize->x + paddingRight - (crossSize / 2) - 1.f,
				categoryHeaderYOffset + textPadding + (textHeight / 2)),
				MC_Color(255, 255, 255), crossWidth, crossSize, !ourWindow->isExtended);
			//DrawUtils::drawRoundRectangle(rectTest, MC_Color(255, 255, 255), true);
			//DrawUtils::drawRoundRectangle(rectTest2, MC_Color(100, 100, 100), true);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	moduleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region Vape
void ClickGui::renderVapeCategory(Category category) {
	const char* categoryName = ClickGui::catToName(category);
	auto clickGUI = moduleMgr->getModule<ClickGUIMod>();

	const shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		}
	}
	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	float currColor[4];  // ArrayList colors
	currColor[3] = rcolors[3];

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset;

	// Get All Modules in our category
	vector<shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);

	// Get max width of all text

	for (auto& it : ModuleList) {
		string len = "saturation   ";
		windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 20);
	}

	vector<shared_ptr<IModule>> moduleList;
	{
		vector<int> toIgniore;
		int moduleCount = (int)ModuleList.size();
		for (int i = 0; i < moduleCount; i++) {
			float bestWidth = 1.f;
			int bestIndex = 1;

			for (int j = 0; j < ModuleList.size(); j++) {
				bool stop = false;
				for (int bruhwth = 0; bruhwth < toIgniore.size(); bruhwth++)
					if (j == toIgniore[bruhwth]) {
						stop = true;
						break;
					}
				if (stop)
					continue;

				string t = ModuleList[j]->getRawModuleName();
				float textWidthRn = DrawUtils::getTextWidth(&t, textSize, Fonts::SMOOTH);
				if (textWidthRn > bestWidth) {
					bestWidth = textWidthRn;
					bestIndex = j;
				}
			}
			moduleList.push_back(ModuleList[bestIndex]);
			toIgniore.push_back(bestIndex);
		}
	}

	const float xEnd = currentXOffset + windowSize->x + paddingRight;
	const float yEnd = currentYOffset + windowSize->y + paddingRight;

	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();

	// Convert mousePos to visual Pos
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;

		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;  // reset scroll
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f)
				ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	// Loop through Modules to display em
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 3.f) + 0.5f /*fix flickering related to rounding errors*/;
		int moduleIndex = 0;
		for (auto& mod : moduleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset)
				continue;
			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			int opacity = 0;
			if (mod->isEnabled()) opacity += 255;
			else opacity -= 0;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave

			if (ourWindow->isInAnimation) {  // Estimate, we don't know about module settings yet
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			string textStr = mod->getRawModuleName();

			vec4_t rectPos = vec4_t(
				currentXOffset,
				currentYOffset,
				xEnd,
				currentYOffset + textHeight + (textPadding * 2));

			string len = "saturatio";
			float lenth = DrawUtils::getTextWidth(&len, textSize) - 3.f;

			vec2_t textPos = vec2_t(
				currentXOffset + lenth,
				currentYOffset + 7);

			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			// Background
			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {  // Is the Mouse hovering above us?
					string tooltip = mod->getTooltip();
					if (clickGUI->showTooltips && !tooltip.empty())
						renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {  // Are we being clicked?
						mod->toggle();
						shouldToggleLeftClick = false;
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
					}
				}
			}

			// Text
			if (allowRender)
			{
				DrawUtils::fillRectangleA(rectPos, mod->isEnabled() ? arrayColor : MC_Color(50, 50, 50, 255));
				DrawUtils::drawCenteredString(textPos, &textStr, textSize, !mod->isEnabled() ? MC_Color(255, 255, 255) : MC_Color(50, 50, 50, 255), false);
				string len = "saturation    ";
				float lenth = DrawUtils::getTextWidth(&len, textSize) + 19;
				vec4_t rectPos2 = vec4_t(rectPos.x + lenth, rectPos.y, rectPos.x + lenth + 10.5f, rectPos.y + 11.5f);
				DrawUtils::fillRectangleA(rectPos2, MC_Color(35, 35, 35, 255));
			}

			// Settings
			{
				vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
					}
					string len = "saturation    ";
					float lenth = DrawUtils::getTextWidth(&len, textSize) + 20;
					DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 3.f, rectPos.x + lenth + 6.f, rectPos.y + 4.f), MC_Color(255, 255, 255, 255));
					DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 5.f, rectPos.x + lenth + 6.f, rectPos.y + 6.f), MC_Color(255, 255, 255, 255));
					DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 7.f, rectPos.x + lenth + 6.f, rectPos.y + 8.f), MC_Color(255, 255, 255, 255));

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						string len = "saturation    ";
						float lenth = DrawUtils::getTextWidth(&len, textSize) + 20;
						vec4_t rectPos2 = vec4_t(rectPos.x + lenth, rectPos.y, rectPos.x + lenth + 10.5f, rectPos.y + 11.5f);
						DrawUtils::fillRectangleA(rectPos2, MC_Color(25, 25, 25, 255));
						DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 3.f, rectPos.x + lenth + 6.f, rectPos.y + 4.f), MC_Color(255, 255, 255, 255));
						DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 5.f, rectPos.x + lenth + 6.f, rectPos.y + 6.f), MC_Color(255, 255, 255, 255));
						DrawUtils::fillRectangleA(vec4_t(rectPos.x + lenth + 5, rectPos.y + 7.f, rectPos.x + lenth + 6.f, rectPos.y + 8.f), MC_Color(255, 255, 255, 255));
						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0)
								continue;

							vec2_t textPos = vec2_t(
								currentXOffset + textPadding + 5,
								currentYOffset + textPadding);

							// Incomplete, because we dont know the endY yet
							vec4_t rectPos = vec4_t(
								currentXOffset + 1,
								currentYOffset,
								xEnd - 1,
								0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									vec4_t boxPos = vec4_t(
										rectPos.z + textPadding - 15,
										rectPos.y + textPadding + 2,
										rectPos.z + textHeight - textPadding - 15,
										rectPos.y + textHeight - textPadding + 2);

									DrawUtils::drawRectangle(boxPos, MC_Color(255, 255, 255), isFocused ? 1 : 0.8f, 0.5f);

									if (setting->value->_bool) {
										DrawUtils::setColor(28, 107, 201, 1);
										boxPos.x += textPadding;
										boxPos.y += textPadding;
										boxPos.z -= textPadding;
										boxPos.w -= textPadding;
										DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
										DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
									}
								}

								// Text
								{
									// Convert first letter to uppercase for more friendlieness
									char name[0x21];
									sprintf_s(name, 0x21, "%s", setting->name);
									if (name[0] != 0) name[0] = toupper(name[0]);

									string elTexto = name;
									string len = "saturation   ";
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 17);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140), true, true, true);
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								// Text and background
								float settingStart = currentYOffset;
								{
									char name[0x22];
									sprintf_s(name, +"%s:", setting->name);
									if (name[0] != 0)
										name[0] = toupper(name[0]);

									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();

									char name2[0x21];
									sprintf_s(name2, 0x21, " %s", i.GetName().c_str());
									if (name2[0] != 0) name2[0] = toupper(name2[0]);
									string elTexto2 = name2;

									string elTexto = string(GRAY) + name + string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight + (textPadding * 2);
									string len = "saturation   ";
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 17);
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true); //enum aids
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char str[10];
									sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									if (name[0] != 0) name[0] = toupper(name[0]);
									string elTexto = name + text;
									string len = "saturation   ";
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 17);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5,
										currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5,
										currentYOffset - textPadding + textHeight + 3.f);
									// Visuals & Logic
									{
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										rect.z = rect.z - 10;
										//rect.z = rect.z - 10;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background
										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress

										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									// Convert first letter to uppercase for more friendlieness
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									if (name[0] != 0)
										name[0] = toupper(name[0]);
									vec2_t textPos = vec2_t(
										rectPos.x + 10,
										rectPos.y + 10);
									char str[10];
									sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									string elTexto = name + text;
									string len = "saturation   ";
									windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										currentXOffset + textPadding + 5,
										currentYOffset + textPadding + 8.5,
										xEnd - textPadding + 5,
										currentYOffset - textPadding + textHeight + 3.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background

										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue)
											value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);

										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight + (textPadding * 2) + 3.f;
								}
							} break;

							default: {
							} break;//I don't know how to remove this. LOL
							}
						} float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight + (textPadding * 2);
			}


			vec4_t winRectPos = vec4_t(
				xOffset,
				yOffset,
				xEnd,
				currentYOffset);

			if (winRectPos.contains(&mousePos)) {
				if (scrollingDirection > 0 && overflowing) {
					ourWindow->yOffset += scrollingDirection;
				}
				else if (scrollingDirection < 0) {
					ourWindow->yOffset += scrollingDirection;
				}
				scrollingDirection = 0;
				if (ourWindow->yOffset < 0) {
					ourWindow->yOffset = 0;
				}
			}
		}
	}

	DrawUtils::flush();
	// Draw Category Header
	{
		string len = "saturatio";
		float lenth = DrawUtils::getTextWidth(&len, textSize);
		vec2_t textPos = vec2_t(
			currentXOffset + lenth,
			categoryHeaderYOffset + 5);

		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin + 0.5f,
			categoryHeaderYOffset - categoryMargin - 3.f,
			currentXOffset + windowSize->x + paddingRight + categoryMargin - 0.5f,
			categoryHeaderYOffset + textHeight + (textPadding * 2));

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && clickGUI->animation == 0) {
					clickGUI->animation = 0.2f;
				}
				else if (!ourWindow->isExtended && clickGUI->animation == 1)
					clickGUI->animation = 0;
				ourWindow->isInAnimation = true;
				if (clickGUI->sounds) {
					auto player = g_Data.getLocalPlayer();
					PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
					level->playSound("random.click", *player->getPos(), 1, 1);
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {  // WE are being dragged
				if (isLeftClickDown) {                                             // Still dragging
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {  // Stopped dragging
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		// Draw component
		{
			//Draw a bit more then just the HudEditor button
			/* {
				std::vector<SettingEntry*>* settings = clickGUI->getSettings();
				string textStr = "Packet";
				float textStrLen = DrawUtils::getTextWidth(&string("------------")) - 2.f;
				float textStrLen2 = DrawUtils::getTextWidth(&string("--------------"));
				float stringLen = DrawUtils::getTextWidth(&textStr) + 2;
				vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
				float mid = windowSize2.x / 2 - 20;

				vec4_t rect = vec4_t(mid, 0, mid + textStrLen, 18);
				vec4_t settingsRect = vec4_t(rect.x + stringLen + 3, rect.y + 2, rect.x + stringLen + 17, rect.y + 16);
				vec4_t hudEditor = vec4_t(rect.x + stringLen + 19, rect.y + 2, rect.x + stringLen + 33, rect.y + 16);

				if (hudEditor.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);

				DrawUtils::fillRectangleA(rect, MC_Color(37, 39, 43, 255));
				DrawUtils::fillRectangleA(settingsRect, MC_Color(9, 12, 16, 255));
				DrawUtils::fillRectangleA(hudEditor, MC_Color(15, 20, 26, 255));
				DrawUtils::drawText(vec2_t(rect.x + 3, rect.y + 4), &textStr, MC_Color(255, 255, 255), 1.f, 1.f, true);

				float ourOffset = 17;
				static bool extended = false;

				if (settingsRect.contains(&mousePos) && shouldToggleRightClick) {
					shouldToggleRightClick = false;
					extended = !extended;
				}

				vec4_t idkRect = vec4_t(settingsRect.x, ourOffset, settingsRect.x + textStrLen2, ourOffset + 16);
				for (int t = 0; t < 4; t++)	idkRect.w += ourOffset;

				if (extended) {
					DrawUtils::fillRectangleA(idkRect, MC_Color(45, 45, 45, 255));
					string stringAids;
					string stringAids2;
					if (clickGUI->theme.getSelectedValue() == 0) stringAids = "Theme: Packet";
					if (clickGUI->theme.getSelectedValue() == 1) stringAids = "Theme: Vape";
					if (clickGUI->theme.getSelectedValue() == 2) stringAids = "Theme: Astolfo";

					if (clickGUI->color.getSelectedValue() == 0) stringAids2 = "Color: Rainbow";
					if (clickGUI->color.getSelectedValue() == 1) stringAids2 = "Color: Astolfo";
					if (clickGUI->color.getSelectedValue() == 2) stringAids2 = "Color: Wave";
					if (clickGUI->color.getSelectedValue() == 3) stringAids2 = "Color: RainbowWave";

					vec4_t selectableSurface = vec4_t(settingsRect.x, ourOffset + 2.f, settingsRect.x + textStrLen2, ourOffset + 17.f);
					vec4_t selectableSurface2 = vec4_t(settingsRect.x, ourOffset + 17.f, settingsRect.x + textStrLen2, ourOffset + 37.f);
					DrawUtils::drawText(vec2_t(selectableSurface.x + 2, selectableSurface.y + 3), &stringAids, MC_Color(255, 255, 255), 1.f, 1.f, true);
					DrawUtils::drawText(vec2_t(selectableSurface2.x + 2, selectableSurface2.y + 3), &stringAids2, MC_Color(255, 255, 255), 1.f, 1.f, true);

					if (selectableSurface.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->theme.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->theme.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->color.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->color.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
				}
			}*/

			//Draw Config Manager button
			{
				auto configManager = moduleMgr->getModule<ConfigManagerMod>();
				string textStr = "Config Menu";
				float infoTextLength = DrawUtils::getTextWidth(&textStr) + DrawUtils::getTextWidth(&string(""), 0.7) + 2;
				static const float height = (1 + 0.7 * 1) * DrawUtils::getFont(Fonts::SMOOTH)->getLineHeight();
				vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
				constexpr float borderPadding = 1;
				float margin = 2.f;
				vec4_t infoRect = vec4_t(
					windowSize2.x - margin - infoTextLength - 2 - borderPadding * 2,
					windowSize2.y - margin - height - 20,
					windowSize2.x - margin + borderPadding - 2,
					windowSize2.y - margin - 2 - 20);
				vec2_t infoTextPos = vec2_t(infoRect.x + 1.5, infoRect.y + 3);
				if (infoRect.contains(&mousePos) && shouldToggleLeftClick) configManager->setEnabled(true);
				if (infoRect.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);
				if (infoRect.contains(&mousePos)) DrawUtils::fillRoundRectangle(infoRect, MC_Color(64, 64, 64, 10), false);
				else DrawUtils::fillRoundRectangle(infoRect, MC_Color(0, 0, 0, 20), false);
				DrawUtils::fillRoundRectangle(infoRect, MC_Color(0, 0, 0, 20), false);
				DrawUtils::drawText(vec2_t(infoTextPos.x + 2, infoTextPos.y), &textStr, MC_Color(255, 255, 255), 1.f, true);
			}

			string period = ".";
			string textStr = categoryName;
			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * 1); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
			DrawUtils::drawCenteredString(textPos, &textStr, textSize, MC_Color(255, 255, 255), false);
			DrawUtils::fillRectangleA(rectPos, MC_Color(35, 35, 35, 255));
			string len = "saturation   ";
			float lenth = DrawUtils::getTextWidth(&len, textSize) + 17;
			vec4_t rectPos2 = vec4_t(rectPos.x + lenth, rectPos.y, rectPos.x + lenth + 10.5f, rectPos.y + 11.5f);
			vec2_t textPosE = vec2_t(rectPos2.x - 5, rectPos2.y + 5);
			string t = "-";
			DrawUtils::drawText(textPosE, &t, MC_Color(255, 255, 255), textSize, true);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) {
			ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		}

		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) {
			ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		}

		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	moduleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region Astolfo
void ClickGui::renderAstolfoCategory(Category category, MC_Color categoryColor, MC_Color categoryColor2) {
	static constexpr float textHeight = textSize * 10.f;
	const char* categoryName = ClickGui::catToName(category);
	auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;

		}
	}

	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset + 2.f;
	vector<shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);
	string len = "saturation           ";
	if (!clickGUI->cFont) len = "saturation    ";
	windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
	const float xEnd = currentXOffset + windowSize->x + paddingRight;
	const float yEnd = currentYOffset + windowSize->y + paddingRight;
	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f)
				ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * ModuleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 3.f) + 0.5f;
		int moduleIndex = 0;
		for (auto& mod : ModuleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave

			auto arrayColor22 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor22 = ColorUtil::rainbowColor(5, 1, 0.7, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor22 = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor22 = ColorUtil::waveColor(clickGUI->r1 - 20, clickGUI->g1 - 20, clickGUI->b1 - 20, clickGUI->r2 - 20, clickGUI->g2 - 20, clickGUI->b2 - 20, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor22 = ColorUtil::RGBWave(clickGUI->r1 - 20, clickGUI->g1 - 20, clickGUI->b1 - 20, clickGUI->r2 - 20, clickGUI->g2 - 20, clickGUI->b2 - 20, ModuleList.size() * moduleIndex * 3); // Wave

			if (clickGUI->categoryColors) arrayColor = categoryColor;
			if (clickGUI->categoryColors) arrayColor22 = categoryColor2;

			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));
			if (ourWindow->isInAnimation) {
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			char name[0x21];
			sprintf_s(name, 0x21, "%s", mod->getRawModuleName());
			string elTexto = name;
			DrawUtils::toLower(elTexto);
			vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec4_t rectPosEnd = vec4_t(xEnd + 10, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec2_t textPos = vec2_t(rectPos.x + 2, currentYOffset + 2);
			vec4_t rectPos2 = vec4_t(currentXOffset + 2.f, yEnd, xEnd - 2.f, currentYOffset + 11.f);
			vec4_t rectPos3 = vec4_t(currentXOffset - 0.5f, yEnd, xEnd + 0.7f, currentYOffset + 12.f);
			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					string tooltip = mod->getTooltip();
					if (clickGUI->showTooltips && !tooltip.empty())
						renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
			}

			if (allowRender)
			{
				shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
				if (!clickMod->isExtended) DrawUtils::fillRectangleA(rectPos, !mod->isEnabled() ? MC_Color(37, 37, 37) : arrayColor);
				DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255));
				string len = "saturation              ";
				float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
				if (!clickGUI->cFont) len = "saturation      ";
				if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
			}

			// Settings
			{
				vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
					}

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended && !ourWindow->isInAnimation) {

						DrawUtils::fillRectangleA(rectPos, MC_Color(27, 27, 27));

						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset + textPadding + 4, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
								string len = "saturation              ";
								float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
								if (!clickGUI->cFont) len = "saturation      ";
								if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									if (setting->value->_bool) {
										vec4_t boxPos = vec4_t(
											rectPos.x + textPadding + 2.5f, rectPos.y,
											rectPos.z + textHeight - textPadding - 11.5,
											rectPos.y + textHeight + 2.5f);
										DrawUtils::fillRectangle(boxPos, arrayColor, isFocused ? 1 : 0.8f);
									}
								}

								// Text
								{
									char name[0x21];
									sprintf_s(name, 0x21, "%s", setting->name);
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140));
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								float settingStart = currentYOffset;
								{
									char name[0x21];
									sprintf_s(name, +"%s:", setting->name);
									string elTexto = string(GRAY) + name;
									DrawUtils::toLower(elTexto);
									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
									char name2[0x21];
									sprintf_s(name2, 0x21, " %s", i.GetName().c_str());
									string elTexto2 = name2;
									DrawUtils::toUpper(elTexto2);
									float lenth2 = DrawUtils::getTextWidth(&elTexto2, textSize) + 2;
									vec2_t textPos22 = vec2_t(rectPos.z - lenth2, rectPos.y + 2);
									elTexto2 = string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight + 2 + (textPadding * 3);
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f); //enum aids
									DrawUtils::drawRightAlignedString(&elTexto2, rectPos, MC_Color(255, 255, 255), false);
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y + 2);
									char str[10];
									sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									string elTexto = name;
									string texto = str;
									DrawUtils::toLower(elTexto);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f);
									DrawUtils::drawRightAlignedString(&texto, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{

									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 2.5f);
									// Visuals & Logic
									{

										rectPos.w += textHeight;
										rect.z = rect.z - 10;
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));

										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10];
									sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize);
									DrawUtils::drawRightAlignedString(&text, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 4.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);

										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue)
											value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6 - 5;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight;
			}

			vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

			if (winRectPos.contains(&mousePos)) {
				if (scrollingDirection > 0 && overflowing) {
					ourWindow->yOffset += scrollingDirection;
				}
				else if (scrollingDirection < 0) {
					ourWindow->yOffset += scrollingDirection;
				}
				scrollingDirection = 0;
				if (ourWindow->yOffset < 0) {
					ourWindow->yOffset = 0;
				}
			}
		}
	}

	DrawUtils::flush();
	{
		vec2_t textPos = vec2_t(currentXOffset + 2, categoryHeaderYOffset + 2);

		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin + 0.5,
			categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin - 0.5,
			categoryHeaderYOffset + textHeight + (textPadding * 2));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : ModuleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		vec4_t rect = vec4_t(
			currentXOffset + textPadding - 1.5f,
			currentYOffset,
			xEnd - textPadding + 2.5f,
			currentYOffset + 1.5);
		if (!clickGUI->cFont) {
			rect = vec4_t(
				currentXOffset + textPadding - 1.5f,
				currentYOffset,
				xEnd - textPadding + 1.5f,
				currentYOffset + 1.5);
		}
		vec4_t rect2 = vec4_t(
			rect.x,
			rect.y,
			rect.z,
			rect.y + 1.f);

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && clickGUI->animation == 0) {
					clickGUI->animation = 0.2f;
				}
				else if (!ourWindow->isExtended && clickGUI->animation == 1) clickGUI->animation = 0;
				ourWindow->isInAnimation = true;
				if (clickGUI->sounds) {
					auto player = g_Data.getLocalPlayer();
					PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
					level->playSound("random.click", *player->getPos(), 1, 1);
				}

				for (auto& mod : ModuleList) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		{
			//Draw a bit more then just the HudEditor button
			/* {
				std::vector<SettingEntry*>* settings = clickGUI->getSettings();
				string textStr = "Packet";
				float textStrLen = DrawUtils::getTextWidth(&string("------------")) - 2.f;
				float textStrLen2 = DrawUtils::getTextWidth(&string("--------------"));
				float stringLen = DrawUtils::getTextWidth(&textStr) + 2;
				vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
				float mid = windowSize2.x / 2 - 20;

				vec4_t rect = vec4_t(mid, 0, mid + textStrLen, 18);
				vec4_t settingsRect = vec4_t(rect.x + stringLen + 3, rect.y + 2, rect.x + stringLen + 17, rect.y + 16);
				vec4_t hudEditor = vec4_t(rect.x + stringLen + 19, rect.y + 2, rect.x + stringLen + 33, rect.y + 16);

				if (hudEditor.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);

				DrawUtils::fillRectangleA(rect, MC_Color(37, 39, 43, 255));
				DrawUtils::fillRectangleA(settingsRect, MC_Color(9, 12, 16, 255));
				DrawUtils::fillRectangleA(hudEditor, MC_Color(15, 20, 26, 255));
				DrawUtils::drawText(vec2_t(rect.x + 3, rect.y + 4), &textStr, MC_Color(255, 255, 255), 1.f, 1.f, true);

				float ourOffset = 17;
				static bool extended = false;

				if (settingsRect.contains(&mousePos) && shouldToggleRightClick) {
					shouldToggleRightClick = false;
					extended = !extended;
				}

				vec4_t idkRect = vec4_t(settingsRect.x, ourOffset, settingsRect.x + textStrLen2, ourOffset + 16);
				for (int t = 0; t < 4; t++)	idkRect.w += ourOffset;

				if (extended) {
					DrawUtils::fillRectangleA(idkRect, MC_Color(45, 45, 45, 255));
					string stringAids;
					string stringAids2;
					if (clickGUI->theme.getSelectedValue() == 0) stringAids = "Theme: Packet";
					if (clickGUI->theme.getSelectedValue() == 1) stringAids = "Theme: Vape";
					if (clickGUI->theme.getSelectedValue() == 2) stringAids = "Theme: Astolfo";

					if (clickGUI->color.getSelectedValue() == 0) stringAids2 = "Color: Rainbow";
					if (clickGUI->color.getSelectedValue() == 1) stringAids2 = "Color: Astolfo";
					if (clickGUI->color.getSelectedValue() == 2) stringAids2 = "Color: Wave";
					if (clickGUI->color.getSelectedValue() == 3) stringAids2 = "Color: RainbowWave";

					vec4_t selectableSurface = vec4_t(settingsRect.x, ourOffset + 2.f, settingsRect.x + textStrLen2, ourOffset + 17.f);
					vec4_t selectableSurface2 = vec4_t(settingsRect.x, ourOffset + 17.f, settingsRect.x + textStrLen2, ourOffset + 37.f);
					DrawUtils::drawText(vec2_t(selectableSurface.x + 2, selectableSurface.y + 3), &stringAids, MC_Color(255, 255, 255), 1.f, 1.f, true);
					DrawUtils::drawText(vec2_t(selectableSurface2.x + 2, selectableSurface2.y + 3), &stringAids2, MC_Color(255, 255, 255), 1.f, 1.f, true);

					if (selectableSurface.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->theme.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->theme.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->color.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->color.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
				}
			}*/

			int moduleIndex = 0;
			for (auto& mod : ModuleList) moduleIndex++;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->categoryColors) arrayColor = categoryColor;

			auto arrayColor2 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor2 = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor2 = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor2 = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor2 = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->categoryColors) arrayColor2 = categoryColor;

			char name[0x21];
			sprintf_s(name, 0x21, "%s", ClickGui::catToName(category));
			name[0] = tolower(name[0]);
			string textStr = name;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255));
			string len = "saturation             ";
			float lenth = DrawUtils::getTextWidth(&len, textSize) + 22.5f;
			if (!clickGUI->cFont) len = "saturation     ";
			if (!clickGUI->cFont) lenth = DrawUtils::getTextWidth(&len, textSize) + 21.5f;
			DrawUtils::fillRectangleA(rectPos, MC_Color(26, 26, 26, 255));
			DrawUtils::drawRoundRectangle(rectTest2, MC_Color(26, 26, 26), false);
			DrawUtils::drawRoundRectangle(rectTest, arrayColor, false);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) {
			ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		}

		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) {
			ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		}

		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	ModuleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region Tenacity
void ClickGui::renderTenacityCategory(Category category, MC_Color categoryColor, MC_Color categoryColor2) {
	static constexpr float textHeight = textSize * 10.f;
	const char* categoryName = ClickGui::catToName(category);
	auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;

		}
	}

	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset + 2.f;
	vector<shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);
	string len = "saturation           ";
	if (!clickGUI->cFont) len = "saturation    ";
	windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&len, textSize) + 16);
	const float xEnd = currentXOffset + windowSize->x + paddingRight;
	const float yEnd = currentYOffset + windowSize->y + paddingRight;
	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f)
				ourWindow->isInAnimation = false;
		}
	}

	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * ModuleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 3.f) + 0.5f;
		int moduleIndex = 0;
		for (auto& mod : ModuleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave

			auto arrayColor22 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor22 = ColorUtil::rainbowColor(5, 1, 0.7, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor22 = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor22 = ColorUtil::waveColor(clickGUI->r1 - 20, clickGUI->g1 - 20, clickGUI->b1 - 20, clickGUI->r2 - 20, clickGUI->g2 - 20, clickGUI->b2 - 20, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor22 = ColorUtil::RGBWave(clickGUI->r1 - 20, clickGUI->g1 - 20, clickGUI->b1 - 20, clickGUI->r2 - 20, clickGUI->g2 - 20, clickGUI->b2 - 20, ModuleList.size() * moduleIndex * 3); // Wave

			if (clickGUI->categoryColors) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->categoryColors) arrayColor22 = categoryColor2;

			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));
			if (ourWindow->isInAnimation) {
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			char name[0x21];
			sprintf_s(name, 0x21, "%s", mod->getRawModuleName());
			string elTexto = name;
			DrawUtils::toLower(elTexto);
			vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec4_t rectPosEnd = vec4_t(xEnd + 10, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec2_t textPos = vec2_t(rectPos.x + 2, currentYOffset + 2);
			vec4_t rectPos2 = vec4_t(currentXOffset + 2.f, yEnd, xEnd - 2.f, currentYOffset + 11.f);
			vec4_t rectPos3 = vec4_t(currentXOffset - 0.5f, yEnd, xEnd + 0.7f, currentYOffset + 12.f);
			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					string tooltip = mod->getTooltip();
					if (clickGUI->showTooltips && !tooltip.empty())
						renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
			}

			if (allowRender)
			{
				shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
				if (!clickMod->isExtended) DrawUtils::fillRectangleA(rectPos, !mod->isEnabled() ? MC_Color(37, 37, 37) : arrayColor);
				DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255));
				string len = "saturation              ";
				float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
				if (!clickGUI->cFont) len = "saturation      ";
				if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
			}

			// Settings
			{
				vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
					}

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended && !ourWindow->isInAnimation) {

						DrawUtils::fillRectangleA(rectPos, MC_Color(27, 27, 27));

						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset + textPadding + 4, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
								string len = "saturation              ";
								float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
								if (!clickGUI->cFont) len = "saturation      ";
								if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									if (setting->value->_bool) {
										vec4_t boxPos = vec4_t(
											rectPos.x + textPadding + 2.5f, rectPos.y,
											rectPos.z + textHeight - textPadding - 11.5,
											rectPos.y + textHeight + 2.5f);
										DrawUtils::fillRectangle(boxPos, arrayColor, isFocused ? 1 : 0.8f);
									}
								}

								// Text
								{
									char name[0x21];
									sprintf_s(name, 0x21, "%s", setting->name);
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140));
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								float settingStart = currentYOffset;
								{
									char name[0x21];
									sprintf_s(name, +"%s:", setting->name);
									string elTexto = string(GRAY) + name;
									DrawUtils::toLower(elTexto);
									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
									char name2[0x21];
									sprintf_s(name2, 0x21, " %s", i.GetName().c_str());
									string elTexto2 = name2;
									DrawUtils::toUpper(elTexto2);
									float lenth2 = DrawUtils::getTextWidth(&elTexto2, textSize) + 2;
									vec2_t textPos22 = vec2_t(rectPos.z - lenth2, rectPos.y + 2);
									elTexto2 = string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight + 2 + (textPadding * 3);
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f); //enum aids
									DrawUtils::drawRightAlignedString(&elTexto2, rectPos, MC_Color(255, 255, 255), false);
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y + 2);
									char str[10];
									sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									string elTexto = name;
									string texto = str;
									DrawUtils::toLower(elTexto);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f);
									DrawUtils::drawRightAlignedString(&texto, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{

									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 2.5f);
									// Visuals & Logic
									{

										rectPos.w += textHeight;
										rect.z = rect.z - 10;
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));

										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10];
									sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize);
									DrawUtils::drawRightAlignedString(&text, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, textSize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, textSize) + 17.5f;
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 4.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);

										DrawUtils::fillRectangleA(rectPos, MC_Color(25, 25, 25, 255));
										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue)
											value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6 - 5;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight;
			}

			vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

			if (winRectPos.contains(&mousePos)) {
				if (scrollingDirection > 0 && overflowing) {
					ourWindow->yOffset += scrollingDirection;
				}
				else if (scrollingDirection < 0) {
					ourWindow->yOffset += scrollingDirection;
				}
				scrollingDirection = 0;
				if (ourWindow->yOffset < 0) {
					ourWindow->yOffset = 0;
				}
			}
		}
	}

	DrawUtils::flush();
	{
		vec2_t textPos = vec2_t(currentXOffset + 2, categoryHeaderYOffset + 2);

		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin + 0.5,
			categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin - 0.5,
			categoryHeaderYOffset + textHeight + (textPadding * 3));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : ModuleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		vec4_t rect = vec4_t(
			currentXOffset + textPadding - 1.5f,
			currentYOffset,
			xEnd - textPadding + 2.5f,
			currentYOffset + 1.5);
		if (!clickGUI->cFont) {
			rect = vec4_t(
				currentXOffset + textPadding - 1.5f,
				currentYOffset,
				xEnd - textPadding + 1.5f,
				currentYOffset + 1.5);
		}
		vec4_t rect2 = vec4_t(
			rect.x,
			rect.y,
			rect.z,
			rect.y + 1.f);

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && clickGUI->animation == 0) {
					clickGUI->animation = 0.2f;
				}
				else if (!ourWindow->isExtended && clickGUI->animation == 1) clickGUI->animation = 0;
				ourWindow->isInAnimation = true;
				if (clickGUI->sounds) {
					auto player = g_Data.getLocalPlayer();
					PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
					level->playSound("random.click", *player->getPos(), 1, 1);
				}

				for (auto& mod : ModuleList) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		{
			//Draw a bit more then just the HudEditor button
			/* {
				std::vector<SettingEntry*>* settings = clickGUI->getSettings();
				string textStr = "Packet";
				float textStrLen = DrawUtils::getTextWidth(&string("------------")) - 2.f;
				float textStrLen2 = DrawUtils::getTextWidth(&string("--------------"));
				float stringLen = DrawUtils::getTextWidth(&textStr) + 2;
				vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
				float mid = windowSize2.x / 2 - 20;

				vec4_t rect = vec4_t(mid, 0, mid + textStrLen, 18);
				vec4_t settingsRect = vec4_t(rect.x + stringLen + 3, rect.y + 2, rect.x + stringLen + 17, rect.y + 16);
				vec4_t hudEditor = vec4_t(rect.x + stringLen + 19, rect.y + 2, rect.x + stringLen + 33, rect.y + 16);

				if (hudEditor.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);

				DrawUtils::fillRectangleA(rect, MC_Color(37, 39, 43, 255));
				DrawUtils::fillRectangleA(settingsRect, MC_Color(9, 12, 16, 255));
				DrawUtils::fillRectangleA(hudEditor, MC_Color(15, 20, 26, 255));
				DrawUtils::drawText(vec2_t(rect.x + 3, rect.y + 4), &textStr, MC_Color(255, 255, 255), 1.f, 1.f, true);

				float ourOffset = 17;
				static bool extended = false;

				if (settingsRect.contains(&mousePos) && shouldToggleRightClick) {
					shouldToggleRightClick = false;
					extended = !extended;
				}

				vec4_t idkRect = vec4_t(settingsRect.x, ourOffset, settingsRect.x + textStrLen2, ourOffset + 16);
				for (int t = 0; t < 4; t++)	idkRect.w += ourOffset;

				if (extended) {
					DrawUtils::fillRectangleA(idkRect, MC_Color(45, 45, 45, 255));
					string stringAids;
					string stringAids2;
					if (clickGUI->theme.getSelectedValue() == 0) stringAids = "Theme: Packet";
					if (clickGUI->theme.getSelectedValue() == 1) stringAids = "Theme: Vape";
					if (clickGUI->theme.getSelectedValue() == 2) stringAids = "Theme: Astolfo";

					if (clickGUI->color.getSelectedValue() == 0) stringAids2 = "Color: Rainbow";
					if (clickGUI->color.getSelectedValue() == 1) stringAids2 = "Color: Astolfo";
					if (clickGUI->color.getSelectedValue() == 2) stringAids2 = "Color: Wave";
					if (clickGUI->color.getSelectedValue() == 3) stringAids2 = "Color: RainbowWave";

					vec4_t selectableSurface = vec4_t(settingsRect.x, ourOffset + 2.f, settingsRect.x + textStrLen2, ourOffset + 17.f);
					vec4_t selectableSurface2 = vec4_t(settingsRect.x, ourOffset + 17.f, settingsRect.x + textStrLen2, ourOffset + 37.f);
					DrawUtils::drawText(vec2_t(selectableSurface.x + 2, selectableSurface.y + 3), &stringAids, MC_Color(255, 255, 255), 1.f, 1.f, true);
					DrawUtils::drawText(vec2_t(selectableSurface2.x + 2, selectableSurface2.y + 3), &stringAids2, MC_Color(255, 255, 255), 1.f, 1.f, true);

					if (selectableSurface.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->theme.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->theme.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->color.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->color.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
				}
			}*/

			int moduleIndex = 0;
			for (auto& mod : ModuleList) moduleIndex++;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			//if (clickGUI->categoryColors) arrayColor = categoryColor;
			arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000);

			auto arrayColor2 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor2 = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor2 = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor2 = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor2 = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->categoryColors) arrayColor2 = categoryColor;

			char name[0x21];
			sprintf_s(name, 0x21, "%s", ClickGui::catToName(category));
			name[0] = tolower(name[0]);
			string textStr = name;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255));
			string len = "saturation             ";
			float lenth = DrawUtils::getTextWidth(&len, textSize) + 22.5f;
			if (!clickGUI->cFont) len = "saturation     ";
			if (!clickGUI->cFont) lenth = DrawUtils::getTextWidth(&len, textSize) + 21.5f;
			DrawUtils::fillRectangleA(rectPos, arrayColor); //MC_Color(26, 26, 26, 255)
			DrawUtils::drawRoundRectangle(rectTest2, MC_Color(26, 26, 26), false);
			DrawUtils::drawRoundRectangle(rectTest, arrayColor, false);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) {
			ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		}

		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) {
			ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		}

		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	ModuleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region tenacity
void ClickGui::rendertenaCategory(Category category) {///
	auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	float textHeight = textSize * clickGUI->txtheight;
	const char* categoryName = ClickGui::catToName(category);
	const shared_ptr<ClickWindow> ourWindow = getWindow(categoryName);

	// Reset Windows to pre-set positions to avoid confusion
	if (resetStartPos && ourWindow->pos.x <= 0) {
		float yot = g_Data.getGuiData()->windowSize.x;
		ourWindow->pos.y = 4;
		switch (category) {
		case Category::COMBAT:
			ourWindow->pos.x = yot / 7.5f;
			break;
		case Category::VISUAL:
			ourWindow->pos.x = yot / 4.1f;
			break;
		case Category::MOVEMENT:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;
		case Category::PLAYER:
			ourWindow->pos.x = yot / 4.25f * 2.f;
			break;
		case Category::EXPLOIT:
			ourWindow->pos.x = yot / 3.4f * 2;
			break;
		case Category::OTHER:
			ourWindow->pos.x = yot / 2.9f * 2.05f;
			break;
		case Category::UNUSED:
			ourWindow->pos.x = yot / 1.6f * 2.2f;
			break;
		case Category::CUSTOM:
			ourWindow->pos.x = yot / 5.6f * 2.f;
			break;

		}
	}

	if (clickGUI->openAnim < 27) ourWindow->pos.y = clickGUI->openAnim;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t* windowSize = &ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset + 2.f;
	vector<shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);
	string len = "saturation           ";
	if (!clickGUI->cFont) len = "saturation    ";



	const float xEnd = currentXOffset + windowSize->x + paddingRight;
	const float yEnd = currentYOffset + windowSize->y + paddingRight;
	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f)
				ourWindow->isInAnimation = false;
		}
	}
	currentYOffset += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= clickGUI->animation * ModuleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf(g_Data.getGuiData()->heightGame * 3.f) + 0.5f;
		int moduleIndex = 0;
		for (auto& mod : ModuleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * 1); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
		//	if (clickGUI->color.getSelectedValue() == 4) arrayColor = ColorUtil::asxRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors
			//if (clickGUI->color.getSelectedValue() == 5) arrayColor = ColorUtil::WeatherRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors

			auto arrayColor22 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * 1); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * 1 * 3); // Wave
		//	if (clickGUI->color.getSelectedValue() == 4) arrayColor = ColorUtil::asxRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors
		//	if (clickGUI->color.getSelectedValue() == 5) arrayColor = ColorUtil::WeatherRainbow(ModuleList.size() * 1 / 2, 1000); // Astolfo Colors


			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));
			if (ourWindow->isInAnimation) {
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > g_Data.getGuiData()->heightGame - 5) {
				overflowing = true;
				break;
			}

			char name[0x21];
			sprintf_s(name, 0x21, "%s", mod->getRawModuleName());
			string elTexto = name;
			DrawUtils::toLower(elTexto);
			vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec4_t rectPosEnd = vec4_t(xEnd + 10, currentYOffset, xEnd, currentYOffset + textHeight + (textPadding * 2));
			vec2_t textPos = vec2_t(currentXOffset + textPadding + 45, currentYOffset + textPadding + 5);
			vec4_t rectPos2 = vec4_t(currentXOffset + 2.f, yEnd, xEnd - 2.f, currentYOffset + 11.f);
			vec4_t rectPos3 = vec4_t(currentXOffset - 0.5f, yEnd, xEnd + 0.7f, currentYOffset + 12.f);
			std::string textStr = mod->getRawModuleName();
			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					string tooltip = mod->getTooltip();
					if (clickGUI->showTooltips && !tooltip.empty())
						renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}
			}

			if (allowRender)
			{
				shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
				if (!clickMod->isExtended) DrawUtils::fillRectangleA(rectPos, !mod->isEnabled() ? MC_Color(57, 57, 57) : arrayColor);
				if (allowRender) DrawUtils::drawCenteredString(textPos, &textStr, clickGUI->txtsize, mod->isEnabled() ? MC_Color(255,255,255) : MC_Color(255, 255, 255), false);
				string len = "saturation              ";
				float lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 20.5f;
				if (!clickGUI->cFont) len = "saturation      ";
				if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 17.5f;
			}

			// Settings
			{
				vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
					}

					currentYOffset += textHeight + (textPadding * 2);

					if (clickMod->isExtended && !ourWindow->isInAnimation) {

						DrawUtils::fillRectangleA(rectPos, MC_Color(27, 27, 27));

						float startYOffset = currentYOffset;
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(currentXOffset + textPadding + 4, currentYOffset + textPadding);
							vec4_t rectPos = vec4_t(currentXOffset, currentYOffset, xEnd, 0);

							if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));
								string len = "saturation              ";
								float lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 20.5f;
								if (!clickGUI->cFont) len = "saturation      ";
								if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 17.5f;
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									if (setting->value->_bool) {
										vec4_t boxPos = vec4_t(
											rectPos.x + textPadding + 2.5f, rectPos.y,
											rectPos.z + textHeight - textPadding - 11.5,
											rectPos.y + textHeight + 2.5f);
										DrawUtils::fillRectangle(boxPos, arrayColor, isFocused ? 1 : 0.8f);
									}
								}

								// Text
								{
									char name[0x21];
									sprintf_s(name, 0x21, "%s", setting->name);
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140));
									currentYOffset += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								float settingStart = currentYOffset;
								{
									char name[0x21];
									sprintf_s(name, +"%s:", setting->name);
									string elTexto = string(GRAY) + name;
									DrawUtils::toLower(elTexto);
									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
									char name2[0x21];
									sprintf_s(name2, 0x21, " %s", i.GetName().c_str());
									string elTexto2 = name2;
									DrawUtils::toUpper(elTexto2);
									float lenth2 = DrawUtils::getTextWidth(&elTexto2, clickGUI->txtsize) + 2;
									vec2_t textPos22 = vec2_t(rectPos.z - lenth2, rectPos.y + 2);
									elTexto2 = string(RESET) + elTexto2;
									rectPos.w = currentYOffset + textHeight + 2 + (textPadding * 3);
									DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), clickGUI->txtsize, 1.f); //enum aids
									DrawUtils::drawRightAlignedString(&elTexto2, rectPos, MC_Color(255, 255, 255), false);
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 17.5f;
									currentYOffset += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								bool isEven = e % 2 == 0;

								rectPos.w = currentYOffset + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = currentYOffset * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								currentYOffset += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y + 2);
									char str[10];
									sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									string elTexto = name;
									string texto = str;
									DrawUtils::toLower(elTexto);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), clickGUI->txtsize, 1.f);
									DrawUtils::drawRightAlignedString(&texto, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 9;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 17.5f;
								}

								if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{

									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 2.5f);
									// Visuals & Logic
									{

										rectPos.w += textHeight;
										rect.z = rect.z - 10;
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));

										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 10);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22];
									sprintf_s(name, "%s: ", setting->name);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10];
									sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									string elTexto = name;
									DrawUtils::toLower(elTexto);
									textPos2.x -= DrawUtils::getTextWidth(&text, clickGUI->txtsize) / 2;
									textPos2.y += 2.5f;
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), clickGUI->txtsize);
									DrawUtils::drawRightAlignedString(&text, rectPos, MC_Color(255, 255, 255), false);
									currentYOffset += textPadding + textHeight - 7;
									rectPos.w = currentYOffset;
									DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));
									string len = "saturation              ";
									float lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 20.5f;
									if (!clickGUI->cFont) len = "saturation      ";
									if (!clickGUI->cFont)lenth = DrawUtils::getTextWidth(&len, clickGUI->txtsize) + 17.5f;
								}
								if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										rectPos.x + textPadding + 2.5f,
										rectPos.y,
										rectPos.z + textHeight - textPadding - 1.5f,
										rectPos.y + textHeight + 4.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = currentYOffset;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);

										DrawUtils::fillRectangleA(rectPos, MC_Color(55, 55, 55, 255));
										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue)
											value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value + 6 - 5;
											DrawUtils::fillRectangle(rect, arrayColor22, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, arrayColor, (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									currentYOffset += textHeight;
								}
							} break;
							}
						}
						float endYOffset = currentYOffset;
					}
				}
				else currentYOffset += textHeight;
			}

			vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd, currentYOffset);

			if (winRectPos.contains(&mousePos)) {
				if (scrollingDirection > 0 && overflowing) {
					ourWindow->yOffset += scrollingDirection;
				}
				else if (scrollingDirection < 0) {
					ourWindow->yOffset += scrollingDirection;
				}
				scrollingDirection = 0;
				if (ourWindow->yOffset < 0) {
					ourWindow->yOffset = 0;
				}
			}
		}
	}

	DrawUtils::flush();
	{
		vec2_t textPos = vec2_t(currentXOffset + 2, categoryHeaderYOffset + 2);

		vec4_t rectPos = vec4_t(
			currentXOffset - categoryMargin + 0.5,
			categoryHeaderYOffset - categoryMargin,
			currentXOffset + windowSize->x + paddingRight + categoryMargin - 0.5,
			categoryHeaderYOffset + textHeight + (textPadding * 2));
		vec4_t rectTest = vec4_t(rectPos.x, rectPos.y + 1, rectPos.z, rectPos.w);
		vec4_t rectTest2 = vec4_t(rectPos.x + 1.f, rectPos.y + 2, rectPos.z - 1.f, rectPos.w);

		for (auto& mod : ModuleList) {
			rectTest.w = currentYOffset - 1;
			rectTest2.w = currentYOffset - 2;
		}

		vec4_t rect = vec4_t(
			currentXOffset + textPadding - 1.5f,
			currentYOffset,
			xEnd - textPadding + 2.5f,
			currentYOffset + 1.5);
		if (!clickGUI->cFont) {
			rect = vec4_t(
				currentXOffset + textPadding - 1.5f,
				currentYOffset,
				xEnd - textPadding + 1.5f,
				currentYOffset + 1.5);
		}
		vec4_t rect2 = vec4_t(
			rect.x,
			rect.y,
			rect.z,
			rect.y + 1.f);

		// Extend Logic
		{
			if (rectPos.contains(&mousePos) && shouldToggleRightClick && !isDragging) {
				shouldToggleRightClick = false;
				ourWindow->isExtended = !ourWindow->isExtended;
				if (ourWindow->isExtended && clickGUI->animation == 0) {
					clickGUI->animation = 0.2f;
				}
				else if (!ourWindow->isExtended && clickGUI->animation == 1) clickGUI->animation = 0;
				ourWindow->isInAnimation = true;
				if (clickGUI->sounds) {
					auto player = g_Data.getLocalPlayer();
					PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
					level->playSound("random.click", *player->getPos(), 1, 1);
				}

				for (auto& mod : ModuleList) {
					shared_ptr<ClickModule> clickMod = getClickModule(ourWindow, mod->getRawModuleName());
					clickMod->isExtended = false;
				}
			}
		}

		// Dragging Logic
		{
			if (isDragging && Utils::getCrcHash(categoryName) == draggedWindow) {
				if (isLeftClickDown) {
					vec2_t diff = vec2_t(mousePos).sub(dragStart);
					ourWindow->pos = ourWindow->pos.add(diff);
					dragStart = mousePos;
				}
				else {
					isDragging = false;
				}
			}
			else if (rectPos.contains(&mousePos) && shouldToggleLeftClick) {
				isDragging = true;
				draggedWindow = Utils::getCrcHash(categoryName);
				shouldToggleLeftClick = false;
				dragStart = mousePos;
			}
		}

		{
			//Draw a bit more then just the HudEditor button
			{
				std::vector<SettingEntry*>* settings = clickGUI->getSettings();
				string textStr = "Radium";
				float textStrLen = DrawUtils::getTextWidth(&string("------------")) - 2.f;
				float textStrLen2 = DrawUtils::getTextWidth(&string("--------------"));
				float stringLen = DrawUtils::getTextWidth(&textStr) + 2;
				vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
				float mid = windowSize2.x / 2 - 20;

				vec4_t rect = vec4_t(mid, 0, mid + textStrLen, 18);
				vec4_t settingsRect = vec4_t(rect.x + stringLen + 3, rect.y + 2, rect.x + stringLen + 17, rect.y + 16);
				vec4_t hudEditor = vec4_t(rect.x + stringLen + 19, rect.y + 2, rect.x + stringLen + 33, rect.y + 16);

				if (hudEditor.contains(&mousePos) && shouldToggleLeftClick) clickGUI->setEnabled(false);

				//DrawUtils::fillRectangleA(rect, MC_Color(37, 39, 43, 255));
				//DrawUtils::fillRectangleA(settingsRect, MC_Color(9, 12, 16, 255));
			//	DrawUtils::fillRectangleA(hudEditor, MC_Color(15, 20, 26, 255));
				DrawUtils::drawText(vec2_t(rect.x + 3, rect.y + 4), &textStr, MC_Color(255, 255, 255), 1.f, 1.f, true);

				float ourOffset = 17;
				static bool extended = false;

				if (settingsRect.contains(&mousePos) && shouldToggleRightClick) {
					shouldToggleRightClick = false;
					extended = !extended;
				}

				vec4_t idkRect = vec4_t(settingsRect.x, ourOffset, settingsRect.x + textStrLen2, ourOffset + 16);
				for (int t = 0; t < 4; t++)	idkRect.w += ourOffset;

				if (extended) {
					DrawUtils::fillRectangleA(idkRect, MC_Color(45, 45, 45, 255));
					string stringAids;
					string stringAids2;
					if (clickGUI->theme.getSelectedValue() == 0) stringAids = "Theme: Packet";
					if (clickGUI->theme.getSelectedValue() == 1) stringAids = "Theme: Vape";
					if (clickGUI->theme.getSelectedValue() == 2) stringAids = "Theme: Astolfo";

					if (clickGUI->color.getSelectedValue() == 0) stringAids2 = "Color: Rainbow";
					if (clickGUI->color.getSelectedValue() == 1) stringAids2 = "Color: Astolfo";
					if (clickGUI->color.getSelectedValue() == 2) stringAids2 = "Color: Wave";
					if (clickGUI->color.getSelectedValue() == 3) stringAids2 = "Color: RainbowWave";

					vec4_t selectableSurface = vec4_t(settingsRect.x, ourOffset + 2.f, settingsRect.x + textStrLen2, ourOffset + 17.f);
					vec4_t selectableSurface2 = vec4_t(settingsRect.x, ourOffset + 17.f, settingsRect.x + textStrLen2, ourOffset + 37.f);
					DrawUtils::drawText(vec2_t(selectableSurface.x + 2, selectableSurface.y + 3), &stringAids, MC_Color(255, 255, 255), 1.f, 1.f, true);
					DrawUtils::drawText(vec2_t(selectableSurface2.x + 2, selectableSurface2.y + 3), &stringAids2, MC_Color(255, 255, 255), 1.f, 1.f, true);

					if (selectableSurface.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->theme.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->theme.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleLeftClick) {
						clickGUI->color.SelectNextValue(false);
						shouldToggleLeftClick = false;
					}
					if (selectableSurface2.contains(&mousePos) && shouldToggleRightClick) {
						clickGUI->color.SelectNextValue(true);
						shouldToggleLeftClick = false;
					}
				}
			}

			int moduleIndex = 0;
			for (auto& mod : ModuleList) moduleIndex++;

			auto arrayColor = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave

			auto arrayColor2 = ColorUtil::rainbowColor(8, 1.F, 1.F, 1);
			if (clickGUI->color.getSelectedValue() == 0) arrayColor2 = ColorUtil::rainbowColor(5, 1, 1, ModuleList.size() * moduleIndex * 2); // Rainbow Colors
			if (clickGUI->color.getSelectedValue() == 1) arrayColor2 = ColorUtil::astolfoRainbow(ModuleList.size() * moduleIndex / 2, 1000); // Astolfo Colors
			if (clickGUI->color.getSelectedValue() == 2) arrayColor2 = ColorUtil::waveColor(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave
			if (clickGUI->color.getSelectedValue() == 3) arrayColor2 = ColorUtil::RGBWave(clickGUI->r1, clickGUI->g1, clickGUI->b1, clickGUI->r2, clickGUI->g2, clickGUI->b2, ModuleList.size() * moduleIndex * 3); // Wave

			char name[0x21];
			sprintf_s(name, 0x21, "%s", ClickGui::catToName(category));
			name[0] = tolower(name[0]);
			string textStr = name;
			DrawUtils::drawText(textPos, &textStr, MC_Color(255, 255, 255));
			string len = "saturation             ";
			float lenth = DrawUtils::getTextWidth(&len, textSize) + 22.5f;
			if (!clickGUI->cFont) len = "saturation     ";
			if (!clickGUI->cFont) lenth = DrawUtils::getTextWidth(&len, textSize) + 21.5f;
			DrawUtils::fillRectangleA(rectPos, MC_Color(arrayColor));
			DrawUtils::drawRoundRectangle(rectTest2, MC_Color(26, 26, 26), false);
			DrawUtils::drawRoundRectangle(rectTest, arrayColor, false);
		}
	}

	// anti idiot
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		if (ourWindow->pos.x + ourWindow->size.x > windowSize.x) {
			ourWindow->pos.x = windowSize.x - ourWindow->size.x;
		}

		if (ourWindow->pos.y + ourWindow->size.y > windowSize.y) {
			ourWindow->pos.y = windowSize.y - ourWindow->size.y;
		}

		ourWindow->pos.x = (float)fmax(0, ourWindow->pos.x);
		ourWindow->pos.y = (float)fmax(0, ourWindow->pos.y);
	}

	ModuleList.clear();
	DrawUtils::flush();
}
#pragma endregion

#pragma region NewGUI

bool combatc = true;
bool renderc = false;
bool movementc = false;
bool playerc = false;
bool exploitc = false;
bool otherc = false;
void ClickGui::renderNewCategory(Category category) {
	static constexpr float textHeight = textSize * 9.f;
	const char* categoryName = ClickGui::catToName(category);
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const std::shared_ptr<ClickWindow2> ourWindow = getWindow2(categoryName);

	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}
	std::string ltx;
	std::string ctx;
	std::string ptx;
	std::string otx;
	std::string etx;
	std::string rtx;
	std::string mtx;
	float zrs = 25;
	float zrsx = 4;
	static auto clickmod = moduleMgr->getModule<ClickGUIMod>();
	static auto inter = moduleMgr->getModule<Interface>();
	if (inter->Fonts.getSelectedValue() == 1)ltx = ("          Movement");
	else ltx = ("      Movement");
	if (inter->Fonts.getSelectedValue() == 1)mtx = ("       Movement");
	else mtx = ("    Movement");
	if (inter->Fonts.getSelectedValue() == 1)ctx = ("       Combat");
	else ctx = ("    Combat");
	if (inter->Fonts.getSelectedValue() == 1)rtx = ("       Render");
	else rtx = ("    Render");
	if (inter->Fonts.getSelectedValue() == 1)ptx = ("       Player");
	else ptx = ("    Player");
	if (inter->Fonts.getSelectedValue() == 1)otx = ("       Other");
	else otx = ("    Other");
	if (inter->Fonts.getSelectedValue() == 1)etx = ("       Exploit");
	else etx = ("    Exploit");

	float tate = 12;
	float yoko = 5.7;
	float sizejack = 1.2f;
	vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
	vec4_t clickGUIRect = vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 + (35 * yoko), windowSize2.y / 2 + (8 * tate));
	DrawUtils::fillRoundRectangle(clickGUIRect, MC_Color(23, 28, 35), true);
	DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH) - 10, windowSize2.y / 2 + (8 * tate)), MC_Color(18, 20, 25), true);
	DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 + (8 * tate)), MC_Color(18, 20, 25), true);

	MC_Color buttoncolor = MC_Color(8, 158, 140);
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 00 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ctx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 10 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 00 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ctx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 10 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = true;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}

	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 10 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&rtx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 20 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 10 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&rtx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 20 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = true;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 20 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&mtx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 30 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 20 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&mtx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 30 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = true;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 30 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ptx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 40 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 30 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ptx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 40 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = true;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 40 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&etx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 50 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 40 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&etx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 50 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = true;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 50 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&otx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 60 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 50 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&otx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 60 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = true;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	DrawUtils::drawImage("textures/ui/icon_recipe_equipment.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));
	DrawUtils::drawImage("textures/gui/newgui/mob_effects/night_vision_effect.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs + 10), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));
	DrawUtils::drawImage("textures/gui/newgui/mob_effects/speed_effect.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs + 20), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));
	DrawUtils::drawImage("textures/ui/icon_steve.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs + 30), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));
	DrawUtils::drawImage("textures/ui/servers.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs + 40), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));
	DrawUtils::drawImage("textures/ui/worldsIcon.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1.5f, windowSize2.y / 2 - (8 * tate) + -1.5f + zrs + 50), vec2_t(10.f, 10.f), vec2_t(0.f, 0.f), vec2_t(1.f, 1.f));

	//DrawUtils::drawRoundRectangle(clickGUIRect, MC_Color(255, 255, 255), true);
	//DrawUtils::drawRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 + (8 * tate)), MC_Color(255, 255, 255),true);
        zrs = 25;
	zrsx = 4;
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx - 1, windowSize2.y / 2 - (8 * tate) + 05), &string("Radium"), MC_Color(0, 170, 0), 2.f, 1.f, true);
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 00 + zrs), &string("   Combat"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 10 + zrs), &string("   Render"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 20 + zrs), &string("   Movement"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 30 + zrs), &string("   Player"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 40 + zrs), &string("   Exploit"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 50 + zrs), &string("   Other"), MC_Color(255, 255, 255));
	DrawUtils::flush();
	const float yOffset = windowSize2.y / 2 - (8 * tate);
	const float xOffset = windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH) + 5;
	vec2_t* windowSize = &ourWindow->size;
	ourWindow->pos.y = windowSize2.y / 2 - (8 * tate);
	ourWindow->pos.x = windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH) + 5;

	float surenobugX = xOffset;
	float surenobugY = yOffset;

	// Get All Modules in our category
	std::vector<std::shared_ptr<IModule>> ModuleList;
	getModuleListByCategory(category, &ModuleList);

	// Get max width of all text
	{
		for (auto& it : ModuleList) {
			std::string label = "-------------";
			windowSize->x = fmax(windowSize->x, DrawUtils::getTextWidth(&label, textSize, Fonts::SMOOTH));
		}
	}

	vector<shared_ptr<IModule>> moduleList;
	{
		vector<int> toIgniore;
		int moduleCount = (int)ModuleList.size();
		for (int i = 0; i < moduleCount; i++) {
			float bestWidth = 1.f;
			int bestIndex = 1;

			for (int j = 0; j < ModuleList.size(); j++) {
				bool stop = false;
				for (int bruhwth = 0; bruhwth < toIgniore.size(); bruhwth++)
					if (j == toIgniore[bruhwth]) {
						stop = true;
						break;
					}
				if (stop)
					continue;

				string t = ModuleList[j]->getRawModuleName();
				float textWidthRn = DrawUtils::getTextWidth(&t, textSize, Fonts::SMOOTH);
				if (textWidthRn > bestWidth) {
					bestWidth = textWidthRn;
					bestIndex = j;
				}
			}
			moduleList.push_back(ModuleList[bestIndex]);
			toIgniore.push_back(bestIndex);
		}
	}

	const float xEnd = windowSize2.x / 2 - 20;

	const float saveModuleXEnd = xEnd;

	float categoryHeaderYOffset = surenobugY;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			clickGUI->animation *= 0.85f;
			if (clickGUI->animation < 0.001f) {
				ourWindow->yOffset = 0;
				ourWindow->isInAnimation = false;
			}

		}
		else {
			clickGUI->animation = 1 - ((1 - clickGUI->animation) * 0.85f);
			if (1 - clickGUI->animation < 0.001f) ourWindow->isInAnimation = false;
		}
	}

	surenobugY += textHeight + (textPadding * 2);
	if (ourWindow->isExtended) {
		if (ourWindow->isInAnimation) {
			surenobugY -= clickGUI->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf((windowSize2.y / 2 + (8 * 12) - 17) * 0.7) + 0.5f;
		int moduleIndex = 0;
		for (auto& mod : moduleList) {
			moduleIndex++;
			if (moduleIndex < ourWindow->yOffset) continue;
			float probableYOffset = (moduleIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			if (ourWindow->isInAnimation) { // Estimate, we don't know about module settings yet
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((surenobugY - ourWindow->pos.y) > cutoffHeight || surenobugY > (windowSize2.y / 2 + (8 * tate) - 16)) {
				overflowing = true;
				break;
			}

			std::string textStr = mod->getRawModuleName();

			vec2_t textPos = vec2_t(surenobugX + textPadding, surenobugY + textPadding);
			vec4_t rectPos = vec4_t(
				surenobugX, surenobugY, xEnd*1.7,
				surenobugY + textHeight + (textPadding * 2)*5.5);

			float animation = 0.f;

			vec4_t rectPosAnim = vec4_t(
				surenobugX, surenobugY, animation,
				surenobugY + textHeight + (textPadding * 2));

			bool allowRender = surenobugY >= categoryHeaderYOffset;

			// Background
			if (allowRender) {
				if (!ourWindow->isInAnimation && !isDragging && rectPos.contains(&mousePos)) {
					static auto clickGuiMod = moduleMgr->getModule<ClickGUIMod>();
					//if (clickGuiMod->showTooltips && !tooltip.empty()) renderTooltip(&tooltip);
					if (shouldToggleLeftClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}

			}
			DrawUtils::fillRoundRectangle(rectPos, MC_Color(18, 20, 25), true);

			// Text
			if (allowRender) {

				vec2_t textPos2 = vec2_t(surenobugX + textPadding, surenobugY + textPadding + 13.5);
				std::string tooltip = mod->getTooltip();
//				xEnd = fmax(fmax(DrawUtils::getTextWidth(&tooltip, textSize, Fonts::SMOOTH), DrawUtils::getTextWidth(&tooltip, textSize, Fonts::SMOOTH)), fmax(DrawUtils::getTextWidth(&textStr, textSize, Fonts::SMOOTH), DrawUtils::getTextWidth(&textStr, textSize, Fonts::SMOOTH)));
				DrawUtils::drawText(textPos, &textStr, mod->isEnabled() ? MC_Color(buttoncolor) : MC_Color(255,255,255), 1.15f, true);
				DrawUtils::drawText(textPos2, &tooltip, MC_Color(100, 100, 100), 1, true);
			}
			float startYsettings = surenobugY;
			int itemIndex = 0;
			surenobugY += textHeight + (textPadding * 2)* 5.5;

			// Settings
			{
				std::vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					std::shared_ptr<ClickModule2> clickMod = getClickModule2(ourWindow, mod->getModuleName());
					float surenobugYfake = surenobugY;
					surenobugYfake += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						for (auto setting : *settings) {
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;
							if ((surenobugYfake - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							string len = "saturation  ";
							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								surenobugYfake += textHeight + (textPadding * 2);
								break;
							}
							case ValueType::ENUM_T: {
								surenobugYfake += textHeight + (textPadding * 2);
								break;
							}
							case ValueType::FLOAT_T: {
								surenobugYfake += textPadding + textHeight - 9;
								surenobugYfake += textHeight + (textPadding * 2) + 3.f;
							} break;
							case ValueType::INT_T: {
								surenobugYfake += textPadding + textHeight - 7;
								surenobugYfake += textHeight + (textPadding * 2) + 3.f;
							} break;
							case ValueType::KEYBIND_T: {
								surenobugYfake += textHeight + (textPadding * 2);
							} break;
						}
						float endYsettings = surenobugYfake;
						DrawUtils::fillRoundRectangle(vec4_t(surenobugX, startYsettings, xEnd*1.7, endYsettings), MC_Color(18, 20, 25), true);
					}
				}
			}
		}


			// Settings
			{
				std::vector<SettingEntry*>* settings = mod->getSettings();
				if (settings->size() > 2 && allowRender) {
					std::shared_ptr<ClickModule2> clickMod = getClickModule2(ourWindow, mod->getModuleName());
					if (rectPos.contains(&mousePos) && shouldToggleRightClick && !ourWindow->isInAnimation) {
						if (clickGUI->sounds) {
							auto player = g_Data.getLocalPlayer();
							PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
							level->playSound("random.click", *player->getPos(), 1, 1);
						}
						shouldToggleRightClick = false;
						clickMod->isExtended = !clickMod->isExtended;
					}

					surenobugY += textHeight + (textPadding * 2);

					if (clickMod->isExtended) {
						float startYOffset = surenobugY;
						for (auto setting : *settings) {
							itemIndex++;
							if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0) continue;

							vec2_t textPos = vec2_t(surenobugX + textPadding + 5, surenobugY + textPadding);
							vec4_t rectPos = vec4_t(surenobugX, surenobugY, xEnd, 0);

							if ((surenobugY - ourWindow->pos.y) > cutoffHeight) {
								overflowing = true;
								break;
							}

							string len = "saturation  ";
							switch (setting->valueType) {
							case ValueType::BOOL_T: {
								rectPos.w = surenobugY + textHeight + (textPadding * 2);
								//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								vec4_t selectableSurface = vec4_t(
									rectPos.x + textPadding,
									rectPos.y + textPadding,
									rectPos.z + textHeight - textPadding - 15,
									rectPos.y + textHeight - textPadding);

								bool isFocused = selectableSurface.contains(&mousePos);
								// Logic
								{
									if (isFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->value->_bool = !setting->value->_bool;
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								// Checkbox
								{
									vec4_t boxPos = vec4_t(
										rectPos.z + textPadding - 15,
										rectPos.y + textPadding + 2,
										rectPos.z + textHeight - textPadding - 15,
										rectPos.y + textHeight - textPadding + 2);

									DrawUtils::drawRectangle(boxPos, MC_Color(255, 255, 255), isFocused ? 1 : 0.8f, 0.5f);

									if (setting->value->_bool) {
										DrawUtils::setColor(28, 107, 201, 1);
										boxPos.x += textPadding;
										boxPos.y += textPadding;
										boxPos.z -= textPadding;
										boxPos.w -= textPadding;
										DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
										DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
									}
								}

								// Text
								{
									// Convert first letter to uppercase for more friendlieness
									char name[0x21]; sprintf_s(name, 0x21, "%s", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									string elTexto = name;

									windowSize->x = fmax(xEnd, DrawUtils::getTextWidth(&len, textSize) + 16);
									DrawUtils::drawText(textPos, &elTexto, isFocused || setting->value->_bool ? MC_Color(255, 255, 255) : MC_Color(140, 140, 140), true, true, true);
									surenobugY += textHeight + (textPadding * 2);
								}
								break;
							}
							case ValueType::ENUM_T: {  // Click setting
								// Text and background
								float settingStart = surenobugY;
								{
									char name[0x22]; sprintf_s(name, +"%s:", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);

									EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();

									char name2[0x21]; sprintf_s(name2, 0x21, " %s", i.GetName().c_str()); if (name2[0] != 0) name2[0] = toupper(name2[0]);
									string elTexto2 = name2;

									string elTexto = string(GRAY) + name + string(RESET) + elTexto2;
									rectPos.w = surenobugY + textHeight - 0.5 + (textPadding * 2);
									windowSize->x = fmax(xEnd, DrawUtils::getTextWidth(&len, textSize) + 16);
									//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true); //enum aids
									surenobugY += textHeight + (textPadding * 2 - 11.5);
								}
								int e = 0;

								if ((surenobugY - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}

								bool isEven = e % 2 == 0;

								rectPos.w = surenobugY + textHeight + (textPadding * 2);
								EnumEntry& i = ((SettingEnum*)setting->extraData)->GetSelectedEntry();
								textPos.y = surenobugY * textPadding;
								vec4_t selectableSurface = vec4_t(textPos.x, textPos.y, xEnd, rectPos.w);
								// logic
								selectableSurface.y = settingStart;
								if (!ourWindow->isInAnimation && selectableSurface.contains(&mousePos)) {
									if (shouldToggleLeftClick) {
										shouldToggleLeftClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(false);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
									else if (shouldToggleRightClick) {
										shouldToggleRightClick = false;
										((SettingEnum*)setting->extraData)->SelectNextValue(true);
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}
									}
								}
								surenobugY += textHeight + (textPadding * 2);

								break;
							}

							case ValueType::FLOAT_T: {
								// Text and background
								{
									vec2_t textPos = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char str[10]; sprintf_s(str, 10, "%.2f", setting->value->_float);
									string text = str;
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									string elTexto = name + text;
									windowSize->x = fmax(xEnd, DrawUtils::getTextWidth(&len, textSize) + 16);
									textPos.x = rectPos.x + 5;
									DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									surenobugY += textPadding + textHeight - 9;
									rectPos.w = surenobugY;
									//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}

								if ((surenobugY - ourWindow->pos.y) > cutoffHeight) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										surenobugX + textPadding + 5, surenobugY + textPadding + 8.5,
										xEnd - textPadding + 5, surenobugY - textPadding + textHeight + 3.f);
									// Visuals & Logic
									{
										rectPos.y = surenobugY;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										rect.z = rect.z - 10;
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										const float minValue = setting->minValue->_float;
										const float maxValue = setting->maxValue->_float - minValue;
										float value = (float)fmax(0, setting->value->_float - minValue);
										if (value > maxValue) value = maxValue; value /= maxValue;
										const float endXlol = (xEnd - textPadding) - (surenobugX + textPadding + 10);
										value *= endXlol;

										{
											rect.z = rect.x + value;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);
										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_float = value;
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									surenobugY += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::INT_T: {
								// Text and background
								{
									// Convert first letter to uppercase for more friendlieness
									vec2_t textPos2 = vec2_t(rectPos.x + ((rectPos.z - rectPos.x) / 2), rectPos.y);
									char name[0x22]; sprintf_s(name, "%s: ", setting->name); if (name[0] != 0) name[0] = toupper(name[0]);
									vec2_t textPos = vec2_t(rectPos.x + 10, rectPos.y + 10);
									char str[10]; sprintf_s(str, 10, "%i", setting->value->_int);
									string text = str;
									textPos2.x -= DrawUtils::getTextWidth(&text, textSize) / 2;
									textPos2.y += 2.5f;
									string elTexto = name + text;
									windowSize->x = fmax(xEnd, DrawUtils::getTextWidth(&len, textSize) + 15);
									textPos2.x = rectPos.x + 5;
									DrawUtils::drawText(textPos2, &elTexto, MC_Color(255, 255, 255), textSize, 1.f, true);
									surenobugY += textPadding + textHeight - 7;
									rectPos.w = surenobugY;
									//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
								}
								if ((surenobugY - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}
								// Slider
								{
									vec4_t rect = vec4_t(
										surenobugX + textPadding + 5,
										surenobugY + textPadding + 8.5,
										xEnd - textPadding + 5,
										surenobugY - textPadding + textHeight + 3.f);


									// Visuals & Logic
									{
										rect.z = rect.z - 10;
										rectPos.y = surenobugY;
										rectPos.w += textHeight + (textPadding * 2) + 3.f;
										// Background
										const bool areWeFocused = rectPos.contains(&mousePos);
										DrawUtils::fillRectangle(rect, MC_Color(150, 150, 150), 1);
										//DrawUtils::fillRectangleA(rectPos, MC_Color(0, 0, 0, clickGUI->opacity));
										DrawUtils::fillRectangle(rect, MC_Color(44, 44, 44), 1);// Background

										const float minValue = (float)setting->minValue->_int;
										const float maxValue = (float)setting->maxValue->_int - minValue;
										float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
										if (value > maxValue) value = maxValue;
										value /= maxValue;  // Value is now in range 0 - 1
										const float endXlol = (xEnd - textPadding) - (surenobugX + textPadding + 5);
										value *= endXlol;  // Value is now pixel diff between start of bar and end of progress
										{
											rect.z = rect.x + value - 5;
											DrawUtils::fillRectangle(rect, MC_Color(255, 255, 255), (areWeFocused || setting->isDragging) ? 1.f : 1.f);

										}

										// Drag Logic
										{
											if (setting->isDragging) {
												if (isLeftClickDown && !isRightClickDown)
													value = mousePos.x - rect.x;
												else
													setting->isDragging = false;
											}
											else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
												shouldToggleLeftClick = false;
												setting->isDragging = true;
												if (clickGUI->sounds) {
													auto player = g_Data.getLocalPlayer();
													PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
													level->playSound("random.click", *player->getPos(), 1, 1);
												}
											}
										}

										// Save Value
										{
											value /= endXlol;  // Now in range 0 - 1
											value *= maxValue;
											value += minValue;

											setting->value->_int = (int)roundf(value);
											setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
										}
									}
									surenobugY += textHeight + (textPadding * 2) + 3.f;
								}
							} break;
							case ValueType::KEYBIND_T: {

								rectPos.w = surenobugY + textHeight + (textPadding * 2);
								if (!isCapturingKey || (keybindMenuCurrent != setting && isCapturingKey)) {
									char name[0x21];
									sprintf_s(name, 0x21, "%s:", setting->name);
									if (name[0] != 0)
										name[0] = toupper(name[0]);

									std::string text = name;

									DrawUtils::drawText(textPos, &text, MC_Color(255, 255, 255), textSize);

									const char* key;

									if (setting->value->_int > 0 && setting->value->_int < 190)
										key = KeyNames[setting->value->_int];
									else if (setting->value->_int == 0x0)
										key = "NONE";
									else
										key = "???";

									if (keybindMenuCurrent == setting && isCapturingKey) {
										key = "...";
									}
									else if (keybindMenuCurrent == setting && isConfirmingKey) {
										if (newKeybind > 0 && newKeybind < 190)
											key = KeyNames[newKeybind];
										else if (newKeybind == 0x0)
											key = "N/A";
										else
											key = "???";
									}

									std::string keyb = key;
									float keybSz = textHeight * 0.8f;

									float length = 10.f;  // because we add 5 to text padding + keybind name
									length += DrawUtils::getTextWidth(&text, textSize);
									length += DrawUtils::getTextWidth(&keyb, textSize);

									//windowSize.x = fmax(windowSize.x, length + offset);

									DrawUtils::drawText(textPos, &text, MC_Color(255, 255, 255), textSize);

									vec2_t textPos2(rectPos.z - 5.f, textPos.y);
									textPos2.x -= DrawUtils::getTextWidth(&keyb, textSize);

									DrawUtils::drawText(textPos2, &keyb, MC_Color(((0.f + 0.f * 1.5f + 0.f) / 3 > 128) ? MC_Color(0, 0, 0) : MC_Color(100, 100, 100)), textSize);
								}
								else {
									std::string text = "Press new bind...";
									//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&text, textSize));

									DrawUtils::drawText(textPos, &text, MC_Color(1.f, 1.f, 1.f), textSize);
								}

								//DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);

								if ((surenobugY - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
									overflowing = true;
									break;
								}

								// Logic
								{
									bool isFocused = rectPos.contains(&mousePos);

									if (isFocused && shouldToggleLeftClick && !(isCapturingKey && keybindMenuCurrent != setting /*don't let the user click other stuff while changing a keybind*/)) {
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}

										keybindMenuCurrent = setting;
										isCapturingKey = true;
									}

									if (isFocused && shouldToggleRightClick && !(isCapturingKey && keybindMenuCurrent != setting)) {
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}

										setting->value->_int = 0x0;  // Clear

										isCapturingKey = false;
									}

									if (shouldStopCapturing && keybindMenuCurrent == setting) {  // The user has selected a key
										if (clickGUI->sounds) {
											auto player = g_Data.getLocalPlayer();
											PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
											level->playSound("random.click", *player->getPos(), 1, 1);
										}

										shouldStopCapturing = false;
										isCapturingKey = false;
										setting->value->_int = newKeybind;
									}
								}
								surenobugY += textHeight + (textPadding * 2);
							} break;
							}
						}
						surenobugY += 10;
					}
				}
				else surenobugY += textHeight + (textPadding * 2);
			}
		}

		vec4_t winRectPos = vec4_t(xOffset, yOffset, xEnd * 1.7, surenobugY);

		if (winRectPos.contains(&mousePos)) {
			if (scrollingDirection > 0 && overflowing) ourWindow->yOffset += scrollingDirection;
			else if (scrollingDirection < 0) ourWindow->yOffset += scrollingDirection;
			scrollingDirection = 0;
			if (ourWindow->yOffset < 0) ourWindow->yOffset = 0;
		}
	}


	moduleList.clear();
	DrawUtils::flush();
}

#pragma endregion
/*
bool combatc = true;
bool renderc = false;
bool movementc = false;
bool playerc = false;
bool exploitc = false;
bool otherc = false;
#pragma region NewGUI
void ClickGui::renderNewCategory(Category category) {
	vec2_t mousePos = *g_Data.getClientInstance()->getMousePos();
	{
		vec2_t windowSize = g_Data.getClientInstance()->getGuiData()->windowSize;
		vec2_t windowSizeReal = g_Data.getClientInstance()->getGuiData()->windowSizeReal;
		mousePos = mousePos.div(windowSizeReal);
		mousePos = mousePos.mul(windowSize);
	}
	std::string ltx;
	float zrs = 25;
	float zrsx = 4;	
	static auto clickmod = moduleMgr->getModule<ClickGUIMod>();
	if (clickmod->Fonts.getSelectedValue()==1)ltx = ("          Movement");
	else ltx = ("      Movement");
	float tate = 12;
	float yoko = 5.7;
	float sizejack = 1.2f;
 	vec2_t windowSize2 = g_Data.getClientInstance()->getGuiData()->windowSize;
	vec4_t clickGUIRect = vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 + (35 * yoko), windowSize2.y / 2 + (8 * tate));
	DrawUtils::fillRoundRectangle(clickGUIRect, MC_Color(23, 28, 35), true);
	DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH)-10, windowSize2.y / 2 + (8 * tate)), MC_Color(18, 20, 25), true);
	DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 + (8 * tate)), MC_Color(18, 20, 25), true);

	MC_Color buttoncolor = MC_Color(8, 158, 140);
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 00 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 10 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 00 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 10 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = true;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}

	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 10 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 20 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 10 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 20 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = true;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 20 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 30 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 20 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 30 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = true;
			playerc = false;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 30 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 40 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 30 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 40 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = true;
			exploitc = false;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 40 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 50 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 40 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 50 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = true;
			otherc = false;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	if (vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 50 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 60 + zrs).contains(&mousePos)) {
		DrawUtils::fillRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate) + 50 + zrs, windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 - (8 * tate) + 60 + zrs), buttoncolor, true);
		if (shouldToggleLeftClick) {
			combatc = false;
			renderc = false;
			movementc = false;
			playerc = false;
			exploitc = false;
			otherc = true;
			if (clickmod->sounds) {
				auto player = g_Data.getLocalPlayer();
				PointingStruct* level = g_Data.getLocalPlayer()->pointingStruct;
				level->playSound("random.click", *player->getPos(), 1, 1);
			}
			shouldToggleLeftClick = false;
		}
	}
	DrawUtils::drawImage("textures/Movement.png", vec2_t(windowSize2.x / 2 - (35 * yoko) + 1, windowSize2.y / 2 - (8 * tate) + -2 + zrs), vec2_t(10.f,10.f), vec2_t(0.f,0.f), vec2_t(1.f, 1.f));
	
	//DrawUtils::drawRoundRectangle(clickGUIRect, MC_Color(255, 255, 255), true);
	//DrawUtils::drawRoundRectangle(vec4_t(windowSize2.x / 2 - (35 * yoko), windowSize2.y / 2 - (8 * tate), windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH), windowSize2.y / 2 + (8 * tate)), MC_Color(255, 255, 255),true);

	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx - 1, windowSize2.y / 2 - (8 * tate) + 05), &string("Radium"), MC_Color(0, 170, 0), 2.f, 1.f, true);
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 00 + zrs), &string("   Combat"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 10 + zrs), &string("   Render"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 20 + zrs), &string("   Movement"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 30 + zrs), &string("   Player"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 40 + zrs), &string("   Exploit"), MC_Color(255, 255, 255));
	DrawUtils::drawText(vec2_t(windowSize2.x / 2 - (35 * yoko) + 1 + zrsx, windowSize2.y / 2 - (8 * tate) + 50 + zrs), &string("   Other"), MC_Color(255, 255, 255));
	DrawUtils::flush();
	static constexpr float textHeight = (textSize * 9.f) * 1.6f;
	const char* categoryName = ClickGui::catToName(category);
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	const std::shared_ptr<ClickWindow2> ourWindow = getWindow2(categoryName);
	ourWindow->pos.y = windowSize2.y / 2 - (8 * tate);
	ourWindow->pos.x = windowSize2.x / 2 - (35 * yoko) + 1 + DrawUtils::getTextWidth(&ltx, 1.f, Fonts::SMOOTH) + 5;

	const float xOffset = ourWindow->pos.x;
	const float yOffset = ourWindow->pos.y;
	vec2_t windowSize = ourWindow->size;
	currentXOffset = xOffset;
	currentYOffset = yOffset;

	struct ClickGuiItem {
		std::shared_ptr<IModule> mod;
		SettingEntry* setting;
		bool isSetting;

		ClickGuiItem(std::shared_ptr<IModule> mod) {
			this->mod = mod;
			isSetting = false;
		}
		ClickGuiItem(SettingEntry* setting) {
			this->setting = setting;
			isSetting = true;
		}
	};
#define TEXTCOLOR (MC_Color(8,158,140))
#define BLACKORWHITE(bright) (MC_Color(bright, bright, bright))

#define FILLCOLOR (MC_Color(0.5f, 0.5f, 0.5f))
#define FILLCOLORDARK (MC_Color(0.5f, 0.5f, 0.5f))
#define FILLCOLORLIGHT (MC_Color(0.5f, 0.5f, 0.5f))

#define FILLCOLORDARK1(a) (MC_Color(0.5f - a, 0.5f - a, 0.5f - a))
#define FILLCOLORLIGHT1(a) (MC_Color(0.5f + a, 0.5f + a, 0.5f + a))

#define FILLCOLORSMORT ((0.2f + 0.2f * 1.5f + 0.2f) / 3 > 128 ? (FILLCOLORDARK) : (FILLCOLORLIGHT))
#define FILLCOLORSMART(a) ((0.2f + 0.2f * 1.5f + 0.2f) / 3 > 128 ? (FILLCOLORDARK1(a)) : (FILLCOLORLIGHT1(a)))

#define OPACITY (0.f)

	// Get All Modules in our category
	std::vector<std::shared_ptr<IModule>> moduleList;
	getModuleListByCategory(category, &moduleList);
	float endXLolFIXed = 0.f;
	// Get max width of all text
	{
		for (auto& it : moduleList) {
			std::string label = "-------------";
			windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&label, textSize * sizejack, Fonts::SMOOTH));
		}
	}


	bool canClick = true;

	{
		bool funny = false;
		for (auto i : windowMap2) {
			if (i.second.get() == ourWindow.get()) {
				funny = true;
				continue;
			}
			if (funny && i.second->selectableSurface.contains(&mousePos))
				canClick = false;
		}
	}

	float categoryHeaderYOffset = currentYOffset;

	if (ourWindow->isInAnimation) {
		if (ourWindow->isExtended) {
			ourWindow->animation *= 0.85f;
			if (ourWindow->animation < 0.001f) {
				ourWindow->yOffset = 0;  // reset scroll
				ourWindow->isInAnimation = false;
			}

		}
		else {
			ourWindow->animation = 1 - ((1 - ourWindow->animation) * 0.85f);
			if (1 - ourWindow->animation < 0.001f)
				ourWindow->isInAnimation = false;
		}
	}
	std::vector<ClickGuiItem> items;

	currentYOffset += textHeight + (textPadding * 2);
	// Loop through Modules to display em
	if (ourWindow->isExtended || ourWindow->isInAnimation) {
		if (ourWindow->isInAnimation) {
			currentYOffset -= ourWindow->animation * moduleList.size() * (textHeight + (textPadding * 2));
		}

		bool overflowing = false;
		const float cutoffHeight = roundf((windowSize2.y / 2 + (8 * tate) - 16) * 0.72f);
		int moduleIndex = 0;

		int itemIndex = 0;

		static auto getSettingSize = [&](SettingEntry* entry) {
			float offset = 2.f * (float)entry->nestValue;

			switch (entry->valueType) {
			case ValueType::BOOL_T: {
				char name[0x21];
				sprintf_s(name, 0x21, "%s", entry->name);
				if (name[0] != 0)
					name[0] = toupper(name[0]);

				std::string elTexto = name;
				return vec2_t(DrawUtils::getTextWidth(&elTexto, textSize) + 10 + offset, 0.f);  // /* because we add 10 to text padding + checkbox
			} break;
			case ValueType::ENUM_SETTING_GROUP_T:
			case ValueType::ENUM_T: {
				float maxLength = 0.f;
				SettingEnum* _enum = (SettingEnum*)entry->extraData;

				// Compute largest enum so that the size doesn't change when you click
				int i = 0;
				for (auto it = _enum->Entrys.begin(); it != _enum->Entrys.end(); it++, i++) {
					maxLength = std::fmaxf(maxLength, DrawUtils::getTextWidth(&it->GetName(), textSize));
				}

				maxLength += 10.f;  // Padding between right side and value + value and name

				char name[0x22];
				sprintf_s(name, "%s:", entry->name);

				if (name[0] != 0x0)
					name[0] = toupper(name[0]);

				std::string text = name;

				maxLength += DrawUtils::getTextWidth(&text, textSize);

				maxLength += offset;

				if (entry->valueType == ValueType::ENUM_SETTING_GROUP_T) {
					for (auto it = _enum->Entrys.begin(); it != _enum->Entrys.end(); it++, i++) {
						std::string text = it->GetName();
						maxLength = fmax(DrawUtils::getTextWidth(&text, textSize) + 7 + offset, maxLength);
					}
				}

				return vec2_t(maxLength, 0.f);
			} break;
			case ValueType::INT_T:
			case ValueType::FLOAT_T: {
				// Convert first letter to uppercase for more friendlieness
				char name[0x22];
				sprintf_s(name, "%s:", entry->name);
				if (name[0] != 0)
					name[0] = toupper(name[0]);

				std::string elTexto = name;
				return vec2_t(DrawUtils::getTextWidth(&elTexto, textSize) + 5 + offset, 0.f);  // /* because we add 5 to text padding
			} break;
			case ValueType::KEYBIND_T: {
				if (!isCapturingKey || (keybindMenuCurrent != entry && isCapturingKey)) {
					char name[0x21];
					sprintf_s(name, 0x21, "%s:", entry->name);
					if (name[0] != 0)
						name[0] = toupper(name[0]);

					std::string text = name;

					const char* key;

					if (entry->value->_int > 0 && entry->value->_int < 190)
						key = KeyNames[entry->value->_int];
					else if (entry->value->_int == 0x0)
						key = "NONE";
					else
						key = "???";

					if (keybindMenuCurrent == entry && isCapturingKey) {
						key = "...";
					}
					else if (keybindMenuCurrent == entry && isConfirmingKey) {
						if (newKeybind > 0 && newKeybind < 190)
							key = KeyNames[newKeybind];
						else if (newKeybind == 0x0)
							key = "N/A";
						else
							key = "???";
					}

					std::string keyb = key;
					float keybSz = textHeight * 0.8f;

					float length = 10.f;  // because we add 5 to text padding + keybind name
					length += DrawUtils::getTextWidth(&text, textSize);
					length += DrawUtils::getTextWidth(&keyb, textSize);

					return vec2_t(length + offset, 0.f);
				}
				else {
					std::string text = "Press new bind...";
					return vec2_t(DrawUtils::getTextWidth(&text, textSize) + offset, 0.f);
				}
			} break;
			default:
				break;
			}

			return vec2_t(0.f, 0.f);
		};


		for (auto it = moduleList.begin(); it != moduleList.end(); ++it) {
			auto mod = *it;
			std::shared_ptr<ClickModule2> clickMod = getClickModule2(ourWindow, mod->getRawModuleName());
			std::vector<SettingEntry*>* settings = mod->getSettings();

			ClickGuiItem module(mod);

			items.push_back(module);

			if (clickMod->isExtended && settings->size() > 2) {
				for (auto it2 = settings->begin(); it2 != settings->end(); ++it2) {
					SettingEntry* setting = *it2;

					if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "Keybind") == 0 || strcmp(setting->name, "Hide in list") == 0)
						continue;

					items.push_back(setting);

					auto newList = setting->getAllExtendedSettings();

					for (auto sett : newList) {
						windowSize.x = fmaxf(getSettingSize(sett).x, windowSize.x);

						items.push_back(sett);
					}

					windowSize.x = fmaxf(getSettingSize(setting).x, windowSize.x);
				}
				SettingEntry* setting = (*settings)[2];
				items.push_back(setting);  // Add the "Visible" setting to the end of the settings.
				windowSize.x = fmaxf(getSettingSize(setting).x, windowSize.x);

				setting = (*settings)[0];
				items.push_back(setting);  // Add our special Keybind setting
				windowSize.x = fmaxf(getSettingSize(setting).x, windowSize.x);
			}
		}

		const float xEnd = currentXOffset + windowSize.x + paddingRight;

		//*
		for (auto& item : items) {
			itemIndex++;

			if (itemIndex < ourWindow->yOffset)
				continue;

			float probableYOffset = (itemIndex - ourWindow->yOffset) * (textHeight + (textPadding * 2));

			if (ourWindow->isInAnimation) {
				if (probableYOffset > cutoffHeight) {
					overflowing = true;
					break;
				}
			}
			else if ((currentYOffset - ourWindow->pos.y) > cutoffHeight || currentYOffset > (windowSize2.y / 2 + (8 * tate) - 16)) {
				overflowing = true;
				break;
			}

			bool allowRender = currentYOffset >= categoryHeaderYOffset;

			if (!item.isSetting) {
				auto mod = item.mod;

				std::shared_ptr<ClickModule2> clickMod = getClickModule2(ourWindow, mod->getRawModuleName());
				std::vector<SettingEntry*>* settings = mod->getSettings();

				std::string textStr = mod->getRawModuleName();

				vec2_t textPos = vec2_t(
					currentXOffset + textPadding,
					currentYOffset + textPadding);

				vec4_t rectPos = vec4_t(
					currentXOffset,
					currentYOffset,
					xEnd * 2,
					(currentYOffset + textHeight + (textPadding * 2) * 2));
				static auto clickGuiMod = moduleMgr->getModule<ClickGUIMod>();

				if (rectPos.contains(&mousePos)) {  // Is the Mouse hovering above us?
					if (shouldToggleLeftClick) {  // Are we being clicked?
						mod->toggle();
						shouldToggleLeftClick = false;
					}
				}

				// Text
				DrawUtils::drawText(textPos, &textStr, mod->isEnabled() ? TEXTCOLOR : BLACKORWHITE(100), textSize * sizejack);
				DrawUtils::drawText(textPos.add(0, 9), &string(mod->getTooltip()), BLACKORWHITE(120), textSize*0.5);

				DrawUtils::fillRoundRectangle(rectPos, MC_Color(18, 20, 25), true);
				if (rectPos.contains(&mousePos) && shouldToggleRightClick) {
					shouldToggleRightClick = false;
					clickMod->isExtended = !clickMod->isExtended;
				}
			} else {
				auto setting = item.setting;

				float offset = 2.f * (float)setting->nestValue;

				setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();

				if (strcmp(setting->name, "enabled") == 0 || strcmp(setting->name, "keybind") == 0)
					continue;

				vec2_t textPos = vec2_t(
					currentXOffset + textPadding + 5 + offset,
					currentYOffset + textPadding);

				// Incomplete, because we dont know the endY yet
				vec4_t rectPos = vec4_t(
					currentXOffset,
					currentYOffset,
					xEnd,
					0);

				if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
					overflowing = true;
					break;
				}

				switch (setting->valueType) {
				case ValueType::BOOL_T: {
					rectPos.w = currentYOffset + textHeight + (textPadding * 2);
					DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);
					vec4_t selectableSurface = vec4_t(
						textPos.x + textPadding,
						textPos.y + textPadding,
						xEnd - textPadding,
						textPos.y + textHeight - textPadding);

					bool isFocused = selectableSurface.contains(&mousePos) && canClick;
					// Logic
					{
						if (isFocused && shouldToggleLeftClick ) {
							shouldToggleLeftClick = false;
							setting->value->_bool = !setting->value->_bool;
						}
					}
					// Checkbox
					{
						vec4_t boxPos = vec4_t(
							textPos.x + textPadding,
							textPos.y + textPadding,
							textPos.x + textHeight - textPadding,
							textPos.y + textHeight - textPadding);

						DrawUtils::drawRectangle(boxPos, BLACKORWHITE(255), isFocused ? 1 : 0.8f, 0.5f);

						if (setting->value->_bool) {
							auto colCross = BLACKORWHITE(255);
							DrawUtils::setColor(colCross.r, colCross.g, colCross.b, 1);
							boxPos.x += textPadding;
							boxPos.y += textPadding;
							boxPos.z -= textPadding;
							boxPos.w -= textPadding;
							DrawUtils::drawLine(vec2_t(boxPos.x, boxPos.y), vec2_t(boxPos.z, boxPos.w), 0.5f);
							DrawUtils::drawLine(vec2_t(boxPos.z, boxPos.y), vec2_t(boxPos.x, boxPos.w), 0.5f);
						}
					}
					textPos.x += textHeight + (textPadding * 2);
					// Text
					{
						// Convert first letter to uppercase for more friendlieness
						char name[0x21];
						sprintf_s(name, 0x21, "%s", setting->name);
						if (name[0] != 0)
							name[0] = toupper(name[0]);

						std::string elTexto = name;
						////windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&elTexto, textSize) + 10 + offset);  // /* because we add 10 to text padding + checkbox
						DrawUtils::drawText(textPos, &elTexto, isFocused ? TEXTCOLOR : BLACKORWHITE(100), textSize);
					}
					break;
				}
				case ValueType::ENUM_SETTING_GROUP_T:
				case ValueType::ENUM_T: {
					// minValue is whether the enum is expanded
					// value is the actual mode (int)

					SettingEnum* _enum = (SettingEnum*)setting->extraData;
					std::string selected = "";  // We are looping through all the values so we might as well set the text beforehand

					// Text and background
					{
						float maxLength = 0.f;

						// Compute largest enum so that the size doesn't change when you click
						int i = 0;
						for (auto it = _enum->Entrys.begin(); it != _enum->Entrys.end(); it++, i++) {
							if (setting->value->_int == i)
								selected = it->GetName();

							maxLength = std::fmaxf(maxLength, DrawUtils::getTextWidth(&it->GetName(), textSize));
						}

						maxLength += 10.f;  // Padding between right side and value + value and name

						char name[0x22];
						sprintf_s(name, "%s:", setting->name);

						if (name[0] != 0x0)
							name[0] = toupper(name[0]);

						std::string text = name;

						maxLength += DrawUtils::getTextWidth(&text, textSize);

						//windowSize.x = fmax(windowSize.x, maxLength + offset);

						DrawUtils::drawText(textPos, &text, BLACKORWHITE(255), textSize);
						rectPos.w = currentYOffset + textHeight + (textPadding * 2);
						if (setting->minValue->_bool)
							currentYOffset += textHeight + textPadding;
						DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);

						vec2_t textPos2 = textPos;

						float textX = rectPos.z - 5.f;
						textX -= DrawUtils::getTextWidth(&selected, textSize);

						textPos2.x = textX;

						DrawUtils::drawText(textPos2, &selected, BLACKORWHITE(100), textSize);
					}

					// Logic
					{
						bool isFocused = rectPos.contains(&mousePos) && canClick;

						if (setting->groups.empty()) {
							if (isFocused && shouldToggleLeftClick) {
								setting->value->_int = (setting->value->_int + 1) % ((SettingEnum*)setting->extraData)->Entrys.size();
								shouldToggleLeftClick = false;
							}
						}
						else {
							SettingGroup* group = setting->groups[setting->value->_int];

							bool isModsExpanded = false;

							if (group != nullptr)
								isModsExpanded = group->isExpanded;

							if (isFocused && shouldToggleLeftClick) {
								if (!isModsExpanded) {
									setting->minValue->_bool = !setting->minValue->_bool;
								}
								else {
									for (auto g : setting->groups) {
										if (g != nullptr)
											g->isExpanded = false;
									}

									setting->minValue->_bool = true;
								}
								shouldToggleLeftClick = false;
							}

							if (isFocused && shouldToggleRightClick && group != nullptr) {
								setting->minValue->_bool = false;
								bool expand = !group->isExpanded;

								for (auto g : setting->groups) {
									if (g != nullptr)
										g->isExpanded = expand;
								}

								shouldToggleRightClick = false;
							}
						}
					}

					// Drop down menu
					if (setting->minValue->_bool) {
						int i = 0;
						for (auto it = _enum->Entrys.begin(); it != _enum->Entrys.end(); it++, i++) {
							bool highlight = i == setting->value->_int;

							textPos.y += textPadding + textHeight;
							if (it + 1 != _enum->Entrys.end())
								currentYOffset += textHeight + textPadding;

							textPos.x += 2;

							std::string text = it->GetName();
							//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&text, textSize) + 7 + offset);
							DrawUtils::drawText(textPos, &text, highlight ? TEXTCOLOR : BLACKORWHITE(100), textSize);

							rectPos.y += textPadding + textHeight;
							rectPos.w += textPadding + textHeight;

							textPos.x -= 2;

							DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);

							// Logic
							if (rectPos.contains(&mousePos) && canClick && shouldToggleLeftClick) {
								setting->value->_int = i;
								shouldToggleLeftClick = false;
							}
						}
					}
				} break;
				case ValueType::FLOAT_T: {
					// Text and background
					{
						// Convert first letter to uppercase for more friendlieness
						char name[0x22];
						sprintf_s(name, "%s:", setting->name);
						if (name[0] != 0)
							name[0] = toupper(name[0]);

						std::string elTexto = name;
						//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&elTexto, textSize) + 5 + offset);  // /* because we add 5 to text padding
						DrawUtils::drawText(textPos, &elTexto, BLACKORWHITE(255), textSize);
						currentYOffset += textPadding + textHeight;
						rectPos.w = currentYOffset;
						DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);
					}

					if ((currentYOffset - ourWindow->pos.y) > cutoffHeight) {
						overflowing = true;
						break;
					}
					// Slider
					{
						vec4_t rect = vec4_t(
							currentXOffset + textPadding + 5 + offset,
							currentYOffset + textPadding,
							xEnd - textPadding,
							currentYOffset - textPadding + textHeight);

						// Visuals & Logic
						{
							rectPos.y = currentYOffset;
							rectPos.w += textHeight + (textPadding * 2);
							// Background
							const bool areWeFocused = rect.contains(&mousePos) && canClick;

							DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);                    // Background
							DrawUtils::drawRectangle(rect, BLACKORWHITE(255), 1.f, 1.f);  // Slider background

							const float minValue = setting->minValue->_float;
							const float maxValue = setting->maxValue->_float - minValue;
							float value = (float)std::fmax(0, setting->value->_float - minValue);  // Value is now always > 0 && < maxValue
							if (value > maxValue)
								value = maxValue;
							value /= maxValue;  // Value is now in range 0 - 1
							const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
							value *= endXlol;  // Value is now pixel diff between start of bar and end of progress

							// Draw Int
							{
								vec2_t mid = vec2_t(
									rect.x + ((rect.z - rect.x) / 2),
									rect.y - 0.2f);
								char str[10];
								sprintf_s(str, 10, "%.2f", setting->value->_float);
								std::string text = str;
								mid.x -= DrawUtils::getTextWidth(&text, textSize) / 2;

								DrawUtils::drawText(mid, &text, BLACKORWHITE(255), textSize);
							}

							// Draw Progress
							{
								rect.z = rect.x + value;
								DrawUtils::fillRectangle(rect, FILLCOLORSMORT, (areWeFocused || setting->isDragging) ? 1.f : 0.8f);
							}

							// Drag Logic
							{
								if (setting->isDragging) {
									if (isLeftClickDown && !isRightClickDown)
										value = mousePos.x - rect.x;
									else
										setting->isDragging = false;
								}
								else if (areWeFocused && shouldToggleLeftClick) {
									shouldToggleLeftClick = false;
									setting->isDragging = true;
								}
							}

							// Save Value
							{
								value /= endXlol;  // Now in range 0 - 1
								value *= maxValue;
								value += minValue;

								setting->value->_float = value;
								setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
							}
						}
					}
				} break;
				case ValueType::INT_T: {
					// Text and background
					{
						// Convert first letter to uppercase for more friendlieness
						char name[0x22];
						sprintf_s(name, "%s:", setting->name);
						if (name[0] != 0)
							name[0] = toupper(name[0]);

						std::string elTexto = name;
						//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&elTexto, textSize) + 5 + offset);  // /* because we add 5 to text padding
						DrawUtils::drawText(textPos, &elTexto, TEXTCOLOR, textSize);
						currentYOffset += textPadding + textHeight;
						rectPos.w = currentYOffset;
						DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);
					}
					if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
						overflowing = true;
						break;
					}
					// Slider
					{
						vec4_t rect = vec4_t(
							currentXOffset + textPadding + 5 + offset,
							currentYOffset + textPadding,
							xEnd - textPadding,
							currentYOffset - textPadding + textHeight);

						// Visuals & Logic
						{
							rectPos.y = currentYOffset;
							rectPos.w += textHeight + (textPadding * 2);
							// Background
							const bool areWeFocused = rect.contains(&mousePos) && canClick;

							DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);                    // Background
							DrawUtils::drawRectangle(rect, BLACKORWHITE(255), 1.f, 1.f);  // Slider background

							const float minValue = (float)setting->minValue->_int;
							const float maxValue = (float)setting->maxValue->_int - minValue;
							float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
							if (value > maxValue)
								value = maxValue;
							value /= maxValue;  // Value is now in range 0 - 1
							const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
							value *= endXlol;  // Value is now pixel diff between start of bar and end of progress

							// Draw Int
							{
								vec2_t mid = vec2_t(
									rect.x + ((rect.z - rect.x) / 2),
									rect.y - 0.2f  // Hardcoded ghetto
								);
								char str[10];
								sprintf_s(str, 10, "%i", setting->value->_int);
								std::string text = str;
								mid.x -= DrawUtils::getTextWidth(&text, textSize) / 2;

								DrawUtils::drawText(mid, &text, BLACKORWHITE(255), textSize);
							}

							// Draw Progress
							{
								rect.z = rect.x + value;
								DrawUtils::fillRectangle(rect, FILLCOLORSMORT, (areWeFocused || setting->isDragging) ? 1.f : 0.8f);
							}

							// Drag Logic
							{
								if (setting->isDragging) {
									if (isLeftClickDown && !isRightClickDown)
										value = mousePos.x - rect.x;
									else
										setting->isDragging = false;
								}
								else if (areWeFocused && shouldToggleLeftClick) {
									shouldToggleLeftClick = false;
									setting->isDragging = true;
								}
							}

							// Save Value
							{
								value /= endXlol;  // Now in range 0 - 1
								value *= maxValue;
								value += minValue;

								setting->value->_int = (int)roundf(value);
								setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
							}
						}
					}
				} break;
				case ValueType::KEYBIND_T: {
					rectPos.w = currentYOffset + textHeight + (textPadding * 2);
					if (!isCapturingKey || (keybindMenuCurrent != setting && isCapturingKey)) {
						char name[0x21];
						sprintf_s(name, 0x21, "%s:", setting->name);
						if (name[0] != 0)
							name[0] = toupper(name[0]);

						std::string text = name;

						DrawUtils::drawText(textPos, &text, TEXTCOLOR, textSize);

						const char* key;

						if (setting->value->_int > 0 && setting->value->_int < 190)
							key = KeyNames[setting->value->_int];
						else if (setting->value->_int == 0x0)
							key = "NONE";
						else
							key = "???";

						if (keybindMenuCurrent == setting && isCapturingKey) {
							key = "...";
						}
						else if (keybindMenuCurrent == setting && isConfirmingKey) {
							if (newKeybind > 0 && newKeybind < 190)
								key = KeyNames[newKeybind];
							else if (newKeybind == 0x0)
								key = "N/A";
							else
								key = "???";
						}

						std::string keyb = key;
						float keybSz = textHeight * 0.8f;

						float length = 10.f;  // because we add 5 to text padding + keybind name
						length += DrawUtils::getTextWidth(&text, textSize);
						length += DrawUtils::getTextWidth(&keyb, textSize);

						//windowSize.x = fmax(windowSize.x, length + offset);

						DrawUtils::drawText(textPos, &text, BLACKORWHITE(255), textSize);

						vec2_t textPos2(rectPos.z - 5.f, textPos.y);
						textPos2.x -= DrawUtils::getTextWidth(&keyb, textSize);

						DrawUtils::drawText(textPos2, &keyb, BLACKORWHITE(100), textSize);
					}
					else {
						std::string text = "Press new bind...";
						//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&text, textSize));

						DrawUtils::drawText(textPos, &text, BLACKORWHITE(255), textSize);
					}

					DrawUtils::fillRectangle(rectPos, FILLCOLOR, OPACITY);

					if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
						overflowing = true;
						break;
					}

					// Logic
					{
						bool isFocused = rectPos.contains(&mousePos) && canClick;

						if (isFocused && shouldToggleLeftClick && !(isCapturingKey && keybindMenuCurrent != setting /*don't let the user click other stuff while changing a keybind)) {
							keybindMenuCurrent = setting;
							isCapturingKey = true;
						}

						if (isFocused && shouldToggleRightClick && !(isCapturingKey && keybindMenuCurrent != setting)) {
							setting->value->_int = 0x0;  // Clear

							isCapturingKey = false;
						}

						if (shouldStopCapturing && keybindMenuCurrent == setting) {  // The user has selected a key
							shouldStopCapturing = false;
							isCapturingKey = false;
							setting->value->_int = newKeybind;
						}
					}
				} break;

					// WIP but I'll keep it here for now
					/*case ValueType::COLOR_PICKER_T: {
						break;  // not implemented yet, gonna work on other stuff
						// TODO: finish this
						{
							// Convert first letter to uppercase for more friendlieness
							char name[0x21];
							sprintf_s(name, 0x21, "%s:", setting->name);
							if (name[0] != 0)
								name[0] = toupper(name[0]);

							std::string elTexto = name;
							//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&elTexto, textSize) + 5);  // /* because we add 5 to text padding
							DrawUtils::drawText(textPos, &elTexto, MC_Color(1.0f, 1.0f, 1.0f), textSize);
							currentYOffset += textPadding + textHeight;
							rectPos.w = currentYOffset;
							DrawUtils::fillRectangle(rectPos, moduleColor, backgroundAlpha);
						}
						if ((currentYOffset - ourWindow->pos.y) > (g_Data.getGuiData()->heightGame * 0.75)) {
							overflowing = true;
							break;
						}

						ColorSettingValue* col = reinterpret_cast<ColorSettingValue*>(setting->value->color);
						MC_Color theColor = col->displayColor;

						// Red slider
						{
							vec4_t rect = vec4_t(
								currentXOffset + textPadding + 5,
								currentYOffset + textPadding,
								xEnd - textPadding,
								currentYOffset - textPadding + textHeight);

							int red = (int)std::floorf(col->displayColor.r * 255);

							// Visuals & Logic
							{
								rectPos.y = currentYOffset;
								rectPos.w += textHeight + (textPadding * 2);
								// Background
								const bool areWeFocused = rect.contains(&mousePos);

								DrawUtils::fillRectangle(rectPos, moduleColor, backgroundAlpha);                   // Background
								DrawUtils::drawRectangle(rect, MC_Color(1.0f, 1.0f, 1.0f), 1.f, backgroundAlpha);  // Slider background

								const float minValue = (float)setting->minValue->_int;
								const float maxValue = (float)setting->maxValue->_int - minValue;
								float value = (float)fmax(0, setting->value->_int - minValue);  // Value is now always > 0 && < maxValue
								if (value > maxValue)
									value = maxValue;
								value /= maxValue;  // Value is now in range 0 - 1
								const float endXlol = (xEnd - textPadding) - (currentXOffset + textPadding + 5);
								value *= endXlol;  // Value is now pixel diff between start of bar and end of progress

								// Draw Int
								{
									vec2_t mid = vec2_t(
										rect.x + ((rect.z - rect.x) / 2),
										rect.y - 0.2f  // Hardcoded ghetto
									);
									char str[10];
									sprintf_s(str, 10, "%i", setting->value->_int);
									std::string text = str;
									mid.x -= DrawUtils::getTextWidth(&text, textSize) / 2;

									DrawUtils::drawText(mid, &text, BLACKORWHITE, textSize);
								}

								// Draw Progress
								{
									rect.z = rect.x + value;
									DrawUtils::fillRectangle(rect, MC_Color(255, 0, 0), (areWeFocused || setting->isDragging) ? 1.f : 0.8f);
								}

								// Drag Logic
								{
									if (setting->isDragging) {
										if (isLeftClickDown && !isRightClickDown)
											value = mousePos.x - rect.x;
										else
											setting->isDragging = false;
									} else if (areWeFocused && shouldToggleLeftClick && !ourWindow->isInAnimation) {
										shouldToggleLeftClick = false;
										setting->isDragging = true;
									}
								}

								// Save Value
								{
									value /= endXlol;  // Now in range 0 - 1
									value *= maxValue;
									value += minValue;

									setting->value->_int = (int)roundf(value);
									setting->makeSureTheValueIsAGoodBoiAndTheUserHasntScrewedWithIt();
								}
							}
						}
					} break;


				default: {
					char alc[100];
					sprintf_s(alc, 100, "Not implemented (%s)", setting->name);
					std::string elTexto = alc;
					// Adjust window size if our text is too  t h i c c
					//windowSize.x = fmax(windowSize.x, DrawUtils::getTextWidth(&elTexto, textSize) + 5 + offset);  // /* because we add 5 to text padding

					DrawUtils::drawText(textPos, &elTexto, MC_Color(255, 0, 0), textSize);
				} break;
				}
			}
			if (!item.isSetting) {
				if (moduleMgr->getModule<ClickGUIMod>()->showTooltips) currentYOffset += (textHeight + (textPadding * 2)) * 2;
				else currentYOffset += textHeight + (textPadding * 2);
			}
			else {
				currentYOffset += textHeight + (textPadding * 2);
				sizejack = 0.f;
			}
		}
		//

		vec4_t winRectPos = vec4_t(
			xOffset,
			yOffset,
			xEnd,
			currentYOffset);

		ourWindow->selectableSurface = winRectPos;

		if (winRectPos.contains(&mousePos)) {
			if (scrollingDirection > 0 && overflowing) {
				ourWindow->yOffset += scrollingDirection;
			}
			else if (scrollingDirection < 0) {
				ourWindow->yOffset += scrollingDirection;
			}
			scrollingDirection = 0;
			if (ourWindow->yOffset < 0) {
				ourWindow->yOffset = 0;
			}
		}
	}

	moduleList.clear();
	DrawUtils::flush();
}*/
#pragma endregion
void ClickGui::render() {
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	if (!moduleMgr->isInitialized()) return;

	if (timesRendered < 1) timesRendered++;

	// Fill Background
	{
		DrawUtils::fillRectangle(vec4_t(0, 0,
			g_Data.getClientInstance()->getGuiData()->widthGame,
			g_Data.getClientInstance()->getGuiData()->heightGame),
			MC_Color(0, 0, 0), 0.3);
	}

	// Render all categorys
	switch (clickGUI->theme.getSelectedValue()) {
	case 0: // Packet
		renderPacketCategory(Category::COMBAT, MC_Color(255, 255, 255));
		renderPacketCategory(Category::VISUAL, MC_Color(255, 255, 255));
		renderPacketCategory(Category::MOVEMENT, MC_Color(255, 255, 255));
		renderPacketCategory(Category::PLAYER, MC_Color(255, 255, 255));
		renderPacketCategory(Category::EXPLOIT, MC_Color(255, 255, 255));
		renderPacketCategory(Category::OTHER, MC_Color(255, 255, 255));
		break;
	case 1: // Vape
		renderVapeCategory(Category::COMBAT);
		renderVapeCategory(Category::VISUAL);
		renderVapeCategory(Category::MOVEMENT);
		renderVapeCategory(Category::PLAYER);
		renderVapeCategory(Category::EXPLOIT);
		renderVapeCategory(Category::OTHER);
		break;
	case 2: // Astolfo
		renderAstolfoCategory(Category::COMBAT, MC_Color(231, 76, 60), MC_Color(231 - 20, 76 - 20, 60 - 20));
		renderAstolfoCategory(Category::VISUAL, MC_Color(55, 0, 206), MC_Color(55 - 20, 0, 206 - 20));
		renderAstolfoCategory(Category::MOVEMENT, MC_Color(46, 204, 113), MC_Color(46 - 20, 204 - 20, 113 - 20));
		renderAstolfoCategory(Category::PLAYER, MC_Color(146, 68, 173), MC_Color(146 - 20, 68 - 20, 173 - 20));
		renderAstolfoCategory(Category::EXPLOIT, MC_Color(52, 152, 219), MC_Color(52 - 20, 152 - 20, 219 - 20));
		renderAstolfoCategory(Category::OTHER, MC_Color(243, 156, 18), MC_Color(243 - 20, 156 - 20, 20 - 20));
		break;
	case 3: // PacketOld
		renderPacketOldCategory(Category::COMBAT, MC_Color(255, 255, 255));
		renderPacketOldCategory(Category::VISUAL, MC_Color(255, 255, 255));
		renderPacketOldCategory(Category::MOVEMENT, MC_Color(255, 255, 255));
		renderPacketOldCategory(Category::PLAYER, MC_Color(255, 255, 255));
		renderPacketOldCategory(Category::EXPLOIT, MC_Color(255, 255, 255));
		renderPacketOldCategory(Category::OTHER, MC_Color(255, 255, 255));
		break;
	case 4: // Tenacity
		renderTenacityCategory(Category::COMBAT, MC_Color(231, 76, 60), MC_Color(231 - 20, 76 - 20, 60 - 20));
		renderTenacityCategory(Category::VISUAL, MC_Color(55, 0, 206), MC_Color(55 - 20, 0, 206 - 20));
		renderTenacityCategory(Category::MOVEMENT, MC_Color(46, 204, 113), MC_Color(46 - 20, 204 - 20, 113 - 20));
		renderTenacityCategory(Category::PLAYER, MC_Color(146, 68, 173), MC_Color(146 - 20, 68 - 20, 173 - 20));
		renderTenacityCategory(Category::EXPLOIT, MC_Color(52, 152, 219), MC_Color(52 - 20, 152 - 20, 219 - 20));
		renderTenacityCategory(Category::OTHER, MC_Color(243, 156, 18), MC_Color(243 - 20, 156 - 20, 20 - 20));
		break;
	case 6: // TANA
		renderTANACategory(Category::COMBAT, MC_Color(255, 255, 255));
		renderTANACategory(Category::VISUAL, MC_Color(255, 255, 255));
		renderTANACategory(Category::MOVEMENT, MC_Color(255, 255, 255));
		renderTANACategory(Category::PLAYER, MC_Color(255, 255, 255));
		renderTANACategory(Category::EXPLOIT, MC_Color(255, 255, 255));
		renderTANACategory(Category::OTHER, MC_Color(255, 255, 255));
		break;
	case 5: // tena2
		if (combatc)renderNewCategory(Category::COMBAT);
		if (renderc)renderNewCategory(Category::VISUAL);
		if (movementc)renderNewCategory(Category::MOVEMENT);
		if (playerc)renderNewCategory(Category::PLAYER);
		if (exploitc)renderNewCategory(Category::EXPLOIT);
		if (otherc)renderNewCategory(Category::OTHER);
		break;
	}

	shouldToggleLeftClick = false;
	shouldToggleRightClick = false;
	resetStartPos = false;
	DrawUtils::flush();
}

void ClickGui::init() { initialised = true; }

void ClickGui::onMouseClickUpdate(int key, bool isDown) {
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();
	if (clickGUI->isEnabled() && g_Data.isInGame()) {
		switch (key) {
		case 1:  // Left Click
			isLeftClickDown = isDown;
			if (isDown)
				shouldToggleLeftClick = true;
			break;
		case 2:  // Right Click
			isRightClickDown = isDown;
			if (isDown)
				shouldToggleRightClick = true;
			break;
		}
	}
}

void ClickGui::onWheelScroll(bool direction) {
	if (!direction) scrollingDirection++;
	else scrollingDirection--;
}

void ClickGui::onKeyUpdate(int key, bool isDown) {
	if (!initialised) return;
	static auto clickGUI = moduleMgr->getModule<ClickGUIMod>();

	if (!isDown) return;

	if (!clickGUI->isEnabled()) {
		timesRendered = 0;
		return;
	}

	if (isCapturingKey && !shouldStopCapturing) {
		if (VK_DELETE == key || VK_ESCAPE == key)
			newKeybind = 0x0;
		else
			newKeybind = key;
		shouldStopCapturing = true;
	}

	if (timesRendered < 1) return;
	timesRendered = 0;

	if (isCapturingKey)
		return;

	switch (key) {
	case VK_ESCAPE:
		clickGUI->setEnabled(false);
		return;
	default:
		if (key == clickGUI->getKeybind()) clickGUI->setEnabled(false);
	}
}

#pragma region Config Stuff
using json = nlohmann::json;
void ClickGui::onLoadConfig(void* confVoid) {
	savedWindowSettings.clear();
	windowMap.clear();
	json* conf = reinterpret_cast<json*>(confVoid);
	if (conf->contains("ClickGuiMenu")) {
		auto obj = conf->at("ClickGuiMenu");
		if (obj.is_null())
			return;
		for (int i = 0; i <= (int)Category::CUSTOM; i++) {
			auto catName = ClickGui::catToName((Category)i);
			if (obj.contains(catName)) {
				auto value = obj.at(catName);
				if (value.is_null())
					continue;
				try {
					SavedWindowSettings windowSettings = {};
					windowSettings.name = catName;
					if (value.contains("pos")) {
						auto posVal = value.at("pos");
						if (!posVal.is_null() && posVal.contains("x") && posVal["x"].is_number_float() && posVal.contains("y") && posVal["y"].is_number_float()) {
							try {
								windowSettings.pos = { posVal["x"].get<float>(), posVal["y"].get<float>() };
							}
							catch (exception e) {
							}
						}
					}
					if (value.contains("isExtended")) {
						auto isExtVal = value.at("isExtended");
						if (!isExtVal.is_null() && isExtVal.is_boolean()) {
							try {
								windowSettings.isExtended = isExtVal.get<bool>();
							}
							catch (exception e) {
							}
						}
					}
					savedWindowSettings[Utils::getCrcHash(catName)] = windowSettings;
				}
				catch (exception e) {
					logF("Config Load Error (ClickGuiMenu): %s", e.what());
				}
			}
		}
	}
}
void ClickGui::onSaveConfig(void* confVoid) {
	json* conf = reinterpret_cast<json*>(confVoid);
	// First update our map
	for (const auto& wind : windowMap) {
		savedWindowSettings[wind.first] = { wind.second->pos, wind.second->isExtended, wind.second->name };
	}

	// Save to json
	if (conf->contains("ClickGuiMenu"))
		conf->erase("ClickGuiMenu");

	json obj = {};

	for (const auto& wind : savedWindowSettings) {
		json subObj = {};
		subObj["pos"]["x"] = wind.second.pos.x;
		subObj["pos"]["y"] = wind.second.pos.y;
		subObj["isExtended"] = wind.second.isExtended;
		obj[wind.second.name] = subObj;
	}

	conf->emplace("ClickGuiMenu", obj);
}
#pragma endregion
