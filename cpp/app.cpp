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
	meshId = MeshMan::INVALID_MMID;
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
	Mat proj = perspective(45, aspect, n, f);
	matrixMan.Set(MatrixMan::PROJ, proj);

	MeshXAnimResult r;
	MeshX* mesh = (MeshX*)meshMan.Get(meshId);
	if (mesh) {
		mesh->CalcAnimation(0, GetTime(), r);
//		mesh->Draw(r, Mat());
		mesh->Draw(r, translate(0, radius * 1.5f, 0) * q2m(Quat(Vec3(0, 0, 1.0f), (float)(GetTime() * M_PI))));
		mesh->Draw(r, translate(radius * 2.0f, 0, 0) * q2m(Quat(Vec3(0, 1.0f, 0), (float)(GetTime() * M_PI))));
	}
	meshRenderer.Flush();
}

void App::Init()
{
//	void LuaBindTest();
//	LuaBindTest();

	ivec2 scrSize = systemMetrics.GetScreenSize();
    glViewport(0, 0, scrSize.x, scrSize.y);

	glClearColor(0.0f, 0.2f, 0.5f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClearDepthf(0);

	meshRenderer.Create();
	waterSurface.Init();
	fontMan.Init();
	luaMan.Create();


	LoadMesh("jiji.x");
}

void App::LoadMesh(const char* fileName)
{
	meshId = meshMan.Create(fileName);
	MeshX* mesh = (MeshX*)meshMan.Get(meshId);
	if (mesh) {
		radius = CalcRadius(mesh);
		float scale = std::max(0.00001f, radius);
		devCamera.SetDistance(scale * 3);
	}
	g_type = "mesh";
}

void App::OnTap(float x, float y)
{
	waterSurface.CreateRipple(Vec2(x, y));
}

void App::Destroy()
{
	luaMan.Destroy();
	texMan.Destroy();
	shaderMan.Destroy();
	waterSurface.Destroy();
	fontMan.Destroy();
	meshRenderer.Destroy();
	meshMan.Destroy();
	meshId = MeshMan::INVALID_MMID;
}

void App::Update()
{
	luaMan.Update();
	waterSurface.Update();
	fps.Update();
	fontMan.DrawString(Vec2(20, 40), 20, SPrintf("FPS: %f", fps.Get()));
}
