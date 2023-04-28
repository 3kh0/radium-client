#include "../PacketClient/Module/ModuleManager.h"
#include "../Utils/TimerUtil.h"
#include "ColorUtil.h"
#include "DrawUtils.h"

MC_Color ColorUtil::rainbowColor(float seconds, float saturation, float brightness, long index) {
	auto interface = moduleMgr->getModule<Interface>();
	float hue6 = (((TimerUtil::getCurrentMs() + index) % (int)(6000)) / (float)(6000));
	float r, g, b = 0;
	Utils::HSVtoRGB(hue6, saturation, brightness, r, g, b);

	return MC_Color(r * 255, g * 255, b * 255, interface->coloropacity);
}

MC_Color ColorUtil::RGBWave(int red, int green, int blue, int red2, int green2, int blue2, long index) {
	auto interface = moduleMgr->getModule<Interface>();
	float hue = ((TimerUtil::getCurrentMs()) % (int)(((int)10) * 1000)) / (float)(((int)10) * 1000);
	float r, g, b = 0;
	Utils::HSVtoRGB(hue, interface->saturation, 1, r, g, b);
	double offset = ((TimerUtil::getCurrentMs() - index) / 2) / (double)120;
	double aids123 = ((TimerUtil::getCurrentMs() - index) % 1000 / 1000.000);
	int aids1234 = ((TimerUtil::getCurrentMs() - index) % 2000 / 2000.000) * 2;
	aids123 = aids1234 % 2 == 0 ? aids123 : 1 - aids123;
	double inverse_percent = 1 - aids123;
	int redPart = (int)(r * red * inverse_percent + r * red2 * aids123);
	int greenPart = (int)(g * green * inverse_percent + g * green2 * aids123);
	int bluePart = (int)(b * blue * inverse_percent + b * blue2 * aids123);

	return MC_Color(redPart, greenPart, bluePart, interface->coloropacity);
}

MC_Color ColorUtil::waveColor(int red, int green, int blue, int red2, int green2, int blue2, long index) {
	auto interface = moduleMgr->getModule<Interface>();
	double offset = ((TimerUtil::getCurrentMs() - index) / 8) / (double)120;
	double aids123 = ((TimerUtil::getCurrentMs() - index) % 1000 / 1000.000);
	int aids1234 = ((TimerUtil::getCurrentMs() - index) % 2000 / 2000.000) * 2;
	aids123 = aids1234 % 2 == 0 ? aids123 : 1 - aids123;
	double inverse_percent = 1 - aids123;
	int redPart = (int)(red * inverse_percent + red2 * aids123);
	int greenPart = (int)(green * inverse_percent + green2 * aids123);
	int bluePart = (int)(blue * inverse_percent + blue2 * aids123);

	return MC_Color(redPart, greenPart, bluePart, interface->coloropacity);
}

MC_Color ColorUtil::astolfoRainbow(int yOffset, int yTotal) {
	auto interface = moduleMgr->getModule<Interface>();
	float speed = 3000;
	float hue = (float)(TimerUtil::getCurrentMs() % (int)speed) + ((yTotal - yOffset) * 9);
	while (hue > speed) { hue -= speed; } hue /= speed;
	if (hue > 0.5) { hue = 0.5F - (hue - 0.5f); } hue += 0.5f;
	float r, g, b = 0;
	Utils::HSVtoRGB(hue, 0.5, 1.f, r, g, b);

	return MC_Color(r * 255, g * 255, b * 255, interface->coloropacity);
}

MC_Color ColorUtil::interfaceColor(int index) {
	auto i = moduleMgr->getModule<Interface>();
	auto color = ColorUtil::rainbowColor(8, 1.F, 1.F, i->coloropacity);

	switch (i->color.getSelectedValue()) {
	case 0: color = ColorUtil::rainbowColor(8, i->saturation, 1.F, -index * 4.f); break; /* Rainbow */
	case 1: color = ColorUtil::astolfoRainbow(index / 5, 1000); break; /* Astolfo */
	case 2: color = ColorUtil::waveColor(i->r, i->g, i->b, i->r2, i->g2, i->b2, index * 3.f); break; /* Wave */
	case 3: color = ColorUtil::RGBWave(i->r, i->g, i->b, i->r2, i->g2, i->b2, index * 3.f); break; /* RGB Wave */
	}
	return MC_Color(color);
}

MC_Color ColorUtil::rainbowColortwo(float seconds, float saturation, float brightness, long index) {
	float hue6 = (((TimerUtil::getCurrentMs() + index) % (int)(6000)) / (float)(6000));
	float r, g, b = 0;
	Utils::HSVtoRGB(hue6, saturation, brightness, r, g, b);
	auto arrayl = moduleMgr->getModule<ArrayList>();
	return MC_Color(r * 255, g * 255, b * 255, arrayl->arraycoloropa);
}

MC_Color ColorUtil::RGBWavetwo(int red, int green, int blue, int red2, int green2, int blue2, long index) {
	auto interface = moduleMgr->getModule<Interface>();
	float hue = ((TimerUtil::getCurrentMs()) % (int)(((int)10) * 1000)) / (float)(((int)10) * 1000);
	float r, g, b = 0;
	Utils::HSVtoRGB(hue, interface->saturation, 1, r, g, b);
	double offset = ((TimerUtil::getCurrentMs() - index) / 2) / (double)120;
	double aids123 = ((TimerUtil::getCurrentMs() - index) % 1000 / 1000.000);
	int aids1234 = ((TimerUtil::getCurrentMs() - index) % 2000 / 2000.000) * 2;
	aids123 = aids1234 % 2 == 0 ? aids123 : 1 - aids123;
	double inverse_percent = 1 - aids123;
	int redPart = (int)(r * red * inverse_percent + r * red2 * aids123);
	int greenPart = (int)(g * green * inverse_percent + g * green2 * aids123);
	int bluePart = (int)(b * blue * inverse_percent + b * blue2 * aids123);
	auto arrayl = moduleMgr->getModule<ArrayList>();
	return MC_Color(redPart, greenPart, bluePart, arrayl->arraycoloropa);
}

MC_Color ColorUtil::waveColortwo(int red, int green, int blue, int red2, int green2, int blue2, long index) {
	double offset = ((TimerUtil::getCurrentMs() - index) / 8) / (double)120;
	double aids123 = ((TimerUtil::getCurrentMs() - index) % 1000 / 1000.000);
	int aids1234 = ((TimerUtil::getCurrentMs() - index) % 2000 / 2000.000) * 2;
	aids123 = aids1234 % 2 == 0 ? aids123 : 1 - aids123;
	double inverse_percent = 1 - aids123;
	int redPart = (int)(red * inverse_percent + red2 * aids123);
	int greenPart = (int)(green * inverse_percent + green2 * aids123);
	int bluePart = (int)(blue * inverse_percent + blue2 * aids123);
	auto arrayl = moduleMgr->getModule<ArrayList>();
	return MC_Color(redPart, greenPart, bluePart, arrayl->arraycoloropa);
}

MC_Color ColorUtil::astolfoRainbowtwo(int yOffset, int yTotal) {
	auto interface = moduleMgr->getModule<Interface>();
	float speed = 3000;
	float hue = (float)(TimerUtil::getCurrentMs() % (int)speed) + ((yTotal - yOffset) * 9);
	while (hue > speed) { hue -= speed; } hue /= speed;
	if (hue > 0.5) { hue = 0.5F - (hue - 0.5f); } hue += 0.5f;
	float r, g, b = 0;
	Utils::HSVtoRGB(hue, 0.5, 1.f, r, g, b);
	auto arrayl = moduleMgr->getModule<ArrayList>();
	return MC_Color(r * 255, g * 255, b * 255, arrayl->arraycoloropa);
}

MC_Color ColorUtil::interfaceColortwo(int index) {
	auto i = moduleMgr->getModule<Interface>();
	auto a = moduleMgr->getModule<ArrayList>();
	auto color = ColorUtil::rainbowColor(8, 1.F, 1.F, a->arraycoloropa);

	switch (i->color.getSelectedValue()) {
	case 0: color = ColorUtil::rainbowColortwo(8, i->saturation, 1.F, -index * 4.f); break; /* Rainbow */
	case 1: color = ColorUtil::astolfoRainbowtwo(index / 5, 1000); break; /* Astolfo */
	case 2: color = ColorUtil::waveColortwo(i->r, i->g, i->b, i->r2, i->g2, i->b2, index * 3.f); break; /* Wave */
	case 3: color = ColorUtil::RGBWavetwo(i->r, i->g, i->b, i->r2, i->g2, i->b2, index * 3.f); break; /* RGB Wave */ }
	return MC_Color(color);
}
