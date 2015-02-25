#include "stdafx.h"

Hub hub;

Hub::Hub()
{

}
void Hub::OnTap(float x, float y)
{
	app.OnTap(x, y);
}

void Hub::Init(int screenW, int screenH)
{
	app.Create();
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
