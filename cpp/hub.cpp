#include "stdafx.h"

Hub hub;

void Hub::Init(int screenW, int screenH)
{
	app.Create();
}

Hub::Hub()
{

}

void Hub::Destroy()
{
	app.Destroy();
}

void Hub::Update(int screenW, int screenH, float offset)
{
	app.Update(screenW, screenH, offset);
	app.Draw();
}
