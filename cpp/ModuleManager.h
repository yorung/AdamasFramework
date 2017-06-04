#pragma once

class AFModule;

class AFModuleManager
{
	std::vector<AFModule*> modules;
public:
	void AddModule(AFModule* module) { modules.push_back(module); }
	void RemoveModule(AFModule* module) { modules.erase(std::remove(modules.begin(), modules.end(), module), modules.end()); }
	void UpdateAll();
	void Draw2DAll(AFCommandList& cmd);
	void Draw3DAll(AFCommandList& cmd);
};

extern AFModuleManager moduleManager;

class AFModule
{
public:
	AFModule() { moduleManager.AddModule(this); }
	virtual ~AFModule() { moduleManager.RemoveModule(this); }
	virtual void Update() {};
	virtual void Draw2D(AFCommandList&) {};
	virtual void Draw3D(AFCommandList&) {};
};
