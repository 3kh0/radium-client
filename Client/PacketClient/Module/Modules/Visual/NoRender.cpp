#include "NoRender.h"
#include "../pch.h"

using namespace std;
NoRender::NoRender() : IModule(0, Category::VISUAL, "Un-Rendering Something") {
    registerBoolSetting("BlockEntities", &blockEntities, blockEntities);
    registerBoolSetting("Particles", &particles, particles);
    registerBoolSetting("Entities", &entities, entities);
}

const char* NoRender::getRawModuleName() {
    return "NoRender";
}

const char* NoRender::getModuleName() {
    return "NoRender";
}
