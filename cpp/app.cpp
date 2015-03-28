#include "stdafx.h"

App app;

std::string g_type;

static float CalcRadius(const Mesh* m)
{
	const Block& b = m->GetRawDatas();
	float maxSq = 0;
	for (auto& it : b.vertices) {
		float sq = lengthSq(it.xyz);
		maxSq = std::max(maxSq, sq);
	}
	return sqrt(maxSq);
}

App::App()
{
	mesh = nullptr;
}

void App::Draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDepthMask(GL_FALSE);
	glDepthFunc(GL_ALWAYS);
	waterSurface.Draw();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_GEQUAL);

	fontMan.Render();

	matrixMan.Set(MatrixMan::WORLD, Mat());
	matrixMan.Set(MatrixMan::VIEW, devCamera.CalcViewMatrix());

	ivec2 scrSize = systemMetrics.GetScreenSize();

	float dist = devCamera.GetDistance();

	float f = dist * 1000;
	float n = dist / 1000;
	float aspect = (float)scrSize.x / scrSize.y;
#ifdef GL_TRUE
	Mat proj = Mat((float)1 / tanf(45 * (float)M_PI / 180 * 0.5f) / aspect, 0, 0, 0,
		0, (float)1 / tanf(45 * (float)M_PI / 180 * 0.5f), 0, 0,
		0, 0, -(f + n) / (f - n), 1,
		0, 0, (n * f) * 2 / (f - n), 0);
#else
	Mat proj = Mat((float)1 / tanf(45 * (float)M_PI / 180 * 0.5f) / aspect, 0, 0, 0,
		0, (float)1 / tanf(45 * (float)M_PI / 180 * 0.5f), 0, 0,
		0, 0, f / (f - n), 1,
		0, 0, -(n * f) / (f - n), 0);
#endif
	matrixMan.Set(MatrixMan::PROJ, proj);

	MeshXAnimResult r;
	mesh->CalcAnimation(0, GetTime(), r);
	mesh->Draw(r);
	meshRenderer.Flush();
}

void App::Init()
{
    ivec2 scrSize = systemMetrics.GetScreenSize();
    glViewport(0, 0, scrSize.x, scrSize.y);

	glClearColor(0.0f, 0.2f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClearDepthf(0);

	meshRenderer.Create();
	waterSurface.Init();
	fontMan.Init();
	LoadMesh("jiji.x");
}

void App::LoadMesh(const char* fileName)
{
	SAFE_DELETE(mesh);
	mesh = new MeshX(fileName);
	float radius = CalcRadius(mesh);
	float scale = std::max(0.00001f, radius);
	devCamera.SetDistance(scale * 3);

	g_type = "mesh";
}

void App::OnTap(float x, float y)
{
	waterSurface.CreateRipple(Vec2(x, y));
}

void App::Destroy()
{
	SAFE_DELETE(mesh);
	texMan.Destroy();
	shaderMan.Destroy();
	waterSurface.Destroy();
	fontMan.Destroy();
	meshRenderer.Destroy();
}

void App::Update()
{
	waterSurface.Update();
	fps.Update();
	fontMan.DrawString(Vec2(20, 20), 20, SPrintf("FPS: %f", fps.Get()));
}
