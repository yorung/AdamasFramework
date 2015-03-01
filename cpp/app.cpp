#include "stdafx.h"

App app;

std::string g_type;

void App::Draw()
{
	glClearColor(1, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	waterSurface.Draw();
	fontMan.Render();
}

void App::Init()
{
	glClearColor(0.0f, 0.2f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glDepthMask(GL_TRUE);
	waterSurface.Init();
	fontMan.Init();
}

void App::OnTap(float x, float y)
{
	waterSurface.CreateRipple(Vec2(x, y));
}

void App::Destroy()
{
	texMan.Destroy();
	shaderMan.Destroy();
	waterSurface.Destroy();
	fontMan.Destroy();
}

void App::Update(int w, int h, float offset)
{
	waterSurface.Update(w, h, offset);
	fps.Update();
	fontMan.DrawString(Vec2(20, 20), 20, SPrintf("FPS: %f", fps.Get()));
}
