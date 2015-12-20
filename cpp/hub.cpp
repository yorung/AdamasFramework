#include "stdafx.h"

Hub hub;

Hub::Hub()
{

}

void Hub::OnTap(float x, float y)
{
	app.OnTap(x, y);
}

void Hub::Init()
{
	app.Init();
}

void Hub::Destroy()
{
	app.Destroy();
}

void Hub::Update()
{
	systemMisc.lastUpdateTime = GetTime();
	app.Update();
	app.Draw();
}
