#include "Killsults.h"

using namespace std;
Killsults::Killsults() : IModule(0, Category::OTHER, "Insults people you kill lol") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Normal", 0);
	mode.addEntry("Sigma", 1);
	mode.addEntry("Funny", 2);
	mode.addEntry("Japanese", 3);
	mode.addEntry("Japanese810", 4);
	mode.addEntry("UwUSpeak", 5);
	mode.addEntry("Health", 6);
	registerBoolSetting("Sound", &sound, sound);
	registerBoolSetting("Notification", &notification, notification);
}

const char* Killsults::getRawModuleName() {
	return "Killsults";
}

const char* Killsults::getModuleName() {
	if (mode.getSelectedValue() == 0) name = string("Killsults ") + string(GRAY) + string("Normal");
	if (mode.getSelectedValue() == 1) name = string("Killsults ") + string(GRAY) + string("Sigma");
	if (mode.getSelectedValue() == 2) name = string("Killsults ") + string(GRAY) + string("Funny");
	if (mode.getSelectedValue() == 3) name = string("Killsults ") + string(GRAY) + string("Japanese");
	if (mode.getSelectedValue() == 4) name = string("Killsults ") + string(GRAY) + string("Japanese810");
	if (mode.getSelectedValue() == 5) name = string("Killsults ") + string(GRAY) + string("UwUSpeak");
	return name.c_str();
}

string normalMessages[32] = {
	"Download Radium today to kick azs while aiding to some abstractional!",
	"I found you in task manager and I ended your process",
	"What's yellow and can't swim? A bus full of children",
	"You are more disappointing than an unsalted pretzel",
	"Take a shower, you smell like your grandpa's toes",
	"You are not Radium Clent approved :rage:",
	"I'm not flying, I'm just defying gravity!",
	"Stop running, you weren't going to win",
	"Knock knock, who's there? Your life",
	"Your client has stopped working",
	"Warning, I have detected haram",
	"I don't hack, I just radium",
	"You should end svchost.exe!",
	"just aided my pantiez",
	"You were an accident",
	"Abstractional Aidz",
	"JACKALOPE TURD BOX",
	"Get dogwatered on",
	"Get 360 No-Scoped",
	"You afraid of me?",
	"Go do the dishes",
	"Job Immediately",
	"Delete System32",
	"I Alt-F4'ed you",
	"Radium is Free",
	"Touch grass",
	"jajajaja",
	"Minority",
	"kkkkkk",
	"clean",
	"idot",
	"F",
};

string cheaterMessages[9] = {
	"How does this bypass ?!?!?",
	"Switch to Radium today!",
	"Violently bhopping I see!",
	"Why Toolbox when Radium?",
	"You probably use Zephyr",
	"Must be using Kek.Club+",
	"SelfHarm Immediately.",
	"Man you're violent",
	"ToolBox moment"
};

string hvhMessages[19] = {
	"Your client has stopped working",
	"Switch to Radium today!",
	"Violently bhopping I see!",
	"Your aim is like derp",
	"You probably use Toolbox",
	"You're so slow, use speed",
	"You're probably playing on VR",
	"Did you know? This server doesn't have Anti-Cheat",
	"Sentinel can only detect speed",
	"Flareon bans legit players",
	"Gwen can't detect most cheats",
	"Why are you flying? oh you were already dead",
	"Press CTRL + L",
	"20 health left",
	"Press Alt + F4 to enable Godmode",
	"you're probably using a $3 mouse",
	"You only have 3IQ",
	"You died too fast",
	"Toolbox moment"
};

string sigmaMessages[2] = {
	"Eat My",
	"Funny Funny Abstractional"
};

string japaneseMessages[13] = {
	"縺顔夢繧型^",
	"莉翫☆縺審adium繧剃ｽｿ縺翫≧!",
	"繧｢繝ｳ繝√メ繝ｼ繝医←縺薙↓縺ゅｋ???",
	"繧ゅ▲縺ｨ鬆ｭ繧剃ｽｿ縺翫≧!",
	"Alt + F4縺ｧ遨ｺ繧帝｣帙∋繧九ｈ!!!",
	"縺ゅ↑縺溘▲縺ｦ縺壹▲縺ｨHive縺励※縺ｾ縺吶ｈ縺ｭ",
	"繧上≠",
	"蠑ｱ縺吶℃縺ｾ縺帙ｓ?",
	"諤偵ｋ縺ｪ縺｣縺ｦw",
	"Celeron縺ｿ縺溘＞縺ｪ閼ｳ縺ｮ謖√■荳ｻ縺ｧ縺吶°?",
	"菴弱せ繝壹↑閼ｳ縺ｿ縺昴ｒ縺頑戟縺｡縺ｮ繧医≧縺ｧ",
	"莉翫☆縺舌↓繝槭う繧ｯ繝ｩ繧貞炎髯､!",
	"莉翫☆縺舌％繧薙↑繧ｴ繝滄ｯ悶°繧画栢縺代ｋ繧薙□!",
	/*"縺翫▲縺ｨ縲∝菅縺ｮ閼ｳ縺ｧ繧ｨ繝ｩ繝ｼ縺檎匱逕溘＠縺ｾ縺励◆",
	"莉翫☆縺仙菅縺ｮ莠ｺ逕溘ｒ蜑企勁",
	"繝槭う繧ｯ繝ｩ髢峨§縺ｦ縺ｯ繧亥ｯ昴ｍ",
	"莉翫☆縺植ntiBanJapan縺ｫ蜿ょ刈!",
	"縺吶∪繧捺焔縺梧ｻ代▲縺溘ｏ",
	"鮟剃ｺｺbypass",
	"荳ｭ闖ｯbypass",
	"逾樣ｯ悶□縺ｪ"*/
};
string japanese810Messages[8] = {
	"縺ｾ縺壹＞縺ｧ縺吶ｈ!",
	"繝輔ぃ!?",
	"繧､繧ｭ繧ｹ繧ｮ繧｣!",
	"114514",
	"縺ゅ▲(蟇溘＠)",
	"繧､繧､繧ｾ繝ｼ繧ｳ繝ｬ",
	"繧､繧ｭ繧ｹ繧ｮ繧､!",
	"繧ｪ繝翫す繝｣繧ｹ",
	/*"縺励ｃ縺ｶ繧後ｈ",
	"縺吶∩縺ｾ縺帙ｓ險ｱ縺励※縺上□縺輔＞!菴輔〒繧ゅ＠縺ｾ縺吶ｓ縺ｧ!",
	"縺ｪ繧薙□縺雁燕",
	"繝後ャ!",
	"縺ｯ縺茨ｼｾ繝ｼ縺吶▲縺斐＞螟ｧ縺阪＞...",
	"繝ｳ繧｢繝ｼ縺｣!"*/
};
string uwuspeakMessage[13] = {
	"Thanks for the fwee woot~",
	"Heyyy OwU!",
	"hehe~~",
	"Thanks for letting me touch you! Hehe!",
	"You're so gentle UwU!",
	"OwO!",
	"Hey! Thanks for letting me kill you~",
	"aahhhhh~",
	"You're so cute!",
	"mmmmmmmm~",
	"You're such a sussy baka",
	"OwO! You're so easy!",
	"I got stuck in the washing machine~"
};

void Killsults::onEnable() {
	killed = false;
}

void Killsults::onPlayerTick(C_Player* plr) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	int random = 0;
	srand(time(NULL));
	if (killed) {
		C_TextPacket textPacket;
		PointingStruct* level = player->pointingStruct;
		vec3_t* pos = player->getPos();
		if (sound) {
			level->playSound("random.orb", *pos, 1, 1);
			level->playSound("firework.blast", *pos, 1, 1);
		}
		if (notification) {
			auto notification = g_Data.addNotification("Killsult:", "Killed player"); notification->duration = 5;
		}
		switch (mode.getSelectedValue()) {
		case 0: // Normal
			random = rand() % 32;
			textPacket.message.setText(normalMessages[random]);
			break;
		case 1: // Sigma
			random = rand() % 2;
			textPacket.message.setText(sigmaMessages[random]);
			break;
		case 2: // HvH
			random = rand() % 19;
			textPacket.message.setText(hvhMessages[random]);
			break;
		case 3: // Japanese
			random = rand() % 13;
			textPacket.message.setText(japaneseMessages[random]);
			break;
		case 4: // Japanese810
			random = rand() % 8;
			textPacket.message.setText(japanese810Messages[random]);
			break;
		case 5: // UwU
			random = rand() % 13;
			textPacket.message.setText(uwuspeakMessage[random]);
			break;
		case 6:
			textPacket.message.setText(to_string(player->getHealth() + player->getAbsorption()) + " Health remaining, Want power? radiumclient.com");
			break;
	}
		textPacket.sourceName.setText(player->getNameTag()->getText());
		textPacket.xboxUserId = to_string(player->getUserId());
		g_Data.getClientInstance()->loopbackPacketSender->sendToServer(&textPacket);
		killed = false;
	}
}
