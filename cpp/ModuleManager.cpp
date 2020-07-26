#include "stdafx.h"
#include "ModuleManager.h"

AFModuleManager moduleManager;

void AFModuleManager::UpdateAll()
{
	std::for_each(modules.begin(), modules.end(), [](AFModule* module) { module->Update(); });
}

void AFModuleManager::Draw3DAll(AFCommandList& cmd, AFRenderTarget& rt, ViewDesc& viewDesc)
{
	std::for_each(modules.begin(), modules.end(), [&cmd, &rt, &viewDesc](AFModule* module) { module->Draw3D(cmd, rt, viewDesc); });
}

void AFModuleManager::Draw2DAll(AFCommandList& cmd, AFRenderTarget& rt)
{
	std::for_each(modules.begin(), modules.end(), [&cmd, &rt](AFModule* module) { module->Draw2D(cmd, rt); });
}
