#include "stdafx.h"

Hub hub;

Hub::Hub()
{

}

void Hub::Init()
{
	app.Init();
}

void Hub::Destroy()
{
	app.Destroy();
#ifdef AF_GLES31
	DiscardIntermediateGLBuffers();
#endif
}

void Hub::Update()
{
	systemMisc.lastUpdateTime = GetTime();
	app.Update();
	app.Draw();
}
