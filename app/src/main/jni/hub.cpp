#include "stdafx.h"

Hub hub;

void Hub::Init(int screenW, int screenH)
{
	glClearColor(0.0f, 0.5f, 1.0f, 1.0f);
}

Hub::Hub()
{

}

void Hub::Destroy()
{
}

void Hub::Update(float aspect, float offset)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
