#include "Step.h"
#include "../pch.h"

using namespace std;
Step::Step() : IModule(0, Category::MOVEMENT, "Increases your step height") {
	registerEnumSetting("Mode", &mode, 0);
	mode.addEntry("Vanilla", 0);
	mode.addEntry("None", 1);
	mode.addEntry("Hive", 2);
	registerBoolSetting("Reverse", &reverse, reverse);
	registerFloatSetting("Height", &height, height, 1.f, 2.f);
}

const char* Step::getRawModuleName() {
	return "Step";
}

const char* Step::getModuleName() {
	if (mode.getSelectedValue() == 0) name = string("Step ") + string(GRAY) + to_string((int)height) + string(".") + to_string((int)(height * 10) - ((int)height * 10));
	if (mode.getSelectedValue() == 1) {
		if (reverse) name = string("Step ") + string(GRAY) + string("Reverse");
		else return "Step";
	}
	return name.c_str();
}

void Step::onTick(C_GameMode* gm) {
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	if (mode.getSelectedValue() == 0) player->stepHeight = height; // nORmal
	else g_Data.getLocalPlayer()->stepHeight = 0.5625f;

	if (reverse && player->onGround && !player->isInWater() && !player->isInLava() && !moduleMgr->getModule<Jesus>()->foundWater)
		player->velocity.y = -1.f;
}
void Step::onMove(C_MoveInputHandler* input) {
	if (mode.getSelectedValue() == 2) 
	{
	auto speedMod = moduleMgr->getModule<Speed>();
	if (!(speedMod->isEnabled() && speedMod->noslabs == true))
	{
		C_LocalPlayer* player = g_Data.getLocalPlayer();
		if (player == nullptr)
			return;

		if (player->isInWater())
			return;

		if (player->isSneaking())
			return;

		vector<vec3_ti> sideBlocks;
		sideBlocks.reserve(8);

		vec2_t moveVec2d = { input->forwardMovement, -input->sideMovement };
		bool pressed = moveVec2d.magnitude() > 0.01f;
		if (!pressed)
			return;
		moveVec2d = moveVec2d.normalized();

		float calcYaw = (player->yaw + 90) * (PI / 180);
		vec3_t moveVec;
		float c = cos(calcYaw);
		float s = sin(calcYaw);
		moveVec2d = { moveVec2d.x * c - moveVec2d.y * s, moveVec2d.x * s + moveVec2d.y * c };

		for (int x = -1; x <= 1; x++) {
			for (int z = -1; z <= 1; z++) {
				if (x == 0 && z == 0)
					continue;

				if (moveVec2d.dot(vec2_t(x, z).normalized()) < 0.6f)
					continue;
				sideBlocks.push_back(vec3_ti(x, 0, z));
			}
		}

		auto pPos = *player->getPos();
		pPos.y = player->aabb.lower.y;
		auto pPosI = vec3_ti(pPos.floor());

		auto isObstructed = [&](int yOff, AABB* obstructingBlock = nullptr, bool ignoreYcoll = false) {
			for (const auto& current : sideBlocks) {
				vec3_ti side = pPosI.add(0, yOff, 0).add(current);
				if (side.y < 0 || side.y >= 256)
					break;
				auto block = player->region->getBlock(side);
				if (block == nullptr || block->blockLegacy == nullptr)
					continue;
				C_BlockLegacy* blockLegacy = block->toLegacy();
				if (blockLegacy == nullptr)
					continue;
				AABB collisionVec;
				if (!blockLegacy->getCollisionShape(&collisionVec, block, player->region, &side, player))
					continue;
				bool intersects = ignoreYcoll ? collisionVec.intersectsXZ(player->aabb.expandedXZ(0.1f)) : collisionVec.intersects(player->aabb.expandedXZ(0.1f));

				if (intersects) {
					if (obstructingBlock != nullptr)
						*obstructingBlock = collisionVec;
					return true;
				}
			}
			return false;
		};

		AABB lowerObsVec, upperObsVec;
		bool upperObstructed = isObstructed(1, &upperObsVec);

		bool lowerObstructed = isObstructed(0, &lowerObsVec);
		float targetSpeed = 5;
		if ((lowerObstructed || upperObstructed)) {
			// simulate because im too lazy to do the math
			const auto distanceError = [](float yVel, float distance) {
				int t = 0;
				constexpr int numIter = 60;
				for (; t < numIter; t++) {
					distance -= yVel;
					yVel -= 0.08f;  // gravity
					yVel *= 0.98f;  // drag

					if (yVel <= 0)
						break;
				}
				return tuple(distance, yVel, t);
			};

			float getOver = lowerObstructed ? lowerObsVec.upper.y : upperObsVec.upper.y;
			{
				if (upperObstructed)
					getOver = max(getOver, upperObsVec.upper.y);
				// max dist that can be reached with our speed

				const int numIterations = (int)ceil(5.f);

				bool lastObstructed = upperObstructed;
				for (int iter = 2; iter < numIterations; iter++) {
					if (isObstructed(iter, &upperObsVec, true)) {
						getOver = max(getOver, upperObsVec.upper.y);
						lastObstructed = true;
					}
					else {
						if (lastObstructed)
							lastObstructed = false;
						else
							break;
					}
				}
			}

			float targetDist = getOver - pPos.y + 0.02f;
			if (targetDist <= 0)
				return;

			auto [curDist, curYVel, curT] = distanceError(player->velocity.y, targetDist);

			//this->clientMessageF("current trajectory error=%.3f t=%i vel=%.3f total=%.2f", curDist, curT, curYVel, targetDist);
			if (curDist <= 0.01f)
				return;  // We will already get on top of the block

			if (player->velocity.y < 5.f) {
				// do another simulation to determine whether we would overshoot on the next iteration
				auto secondTrajectory = distanceError(5, targetDist);
				//this->clientMessageF("secondTrajectory: error=%.3f t=%i vel=%.2f", get<0>(secondTrajectory), get<2>(secondTrajectory), get<1>(secondTrajectory));
				if (get<0>(secondTrajectory) <= 0) {  // we are overshooting if we give the player our target speed

					// use secant method to approximate perfect start speed
					float error = curDist;
					float startSpeed = player->velocity.y;

					float error2 = get<0>(secondTrajectory);
					float startSpeed2 = 5.f;
					int i = 0;
					for (; i < 16; i++) {
						if (error > -0.05f && error <= 0.001f)
							break;  // its better to slightly overshoot than to undershoot

						float newSpeed = (startSpeed2 * error - startSpeed * error2) / (error - error2);
						startSpeed2 = startSpeed;
						startSpeed = newSpeed;

						error2 = error;
						error = get<0>(distanceError(newSpeed, targetDist));
					}
					//this->clientMessageF("Secant method finished with error=%.3f speed=%.3f at t=%i", error, startSpeed, i);
					targetSpeed = startSpeed;
				}
			}
		}
		if (upperObstructed || lowerObstructed) {
			if (player->velocity.y < targetSpeed && player->onGround) {
				vec3_t pos = *player->getPos();
				player->jumpFromGround();
				player->setPos(vec3_t(pos.x, pos.y + 0.5f, pos.z));
				player->velocity.y = 0;
			}
		}
	}
	
	}
}

void Step::onDisable() {
	g_Data.getClientInstance()->minecraft->setTimerSpeed(20.f);
	auto player = g_Data.getLocalPlayer();
	if (player == nullptr) return;

	player->stepHeight = 0.5625f;
}
