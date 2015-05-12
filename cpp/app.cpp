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
	afDepthStencilMode(true);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	waterSurface.Draw();
/*
	AFRenderTarget rt;
	afSetRenderTarget(rt);
	waterSurface.Draw();
	afSetRenderTarget(nullptr);
	afDrawFullScrenEffect("letterBox", rc);
*/
	afDepthStencilMode(true);
	afBlendMode(BM_NONE);

	matrixMan.Set(MatrixMan::VIEW, devCamera.CalcViewMatrix());

	ivec2 scrSize = systemMetrics.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	float dist = devCamera.GetDistance();

	float f = dist * 1000;
	float n = dist / 1000;
	float aspect = (float)scrSize.x / scrSize.y;
	Mat proj = perspective(45, aspect, n, f);
	matrixMan.Set(MatrixMan::PROJ, proj);
/*
	MeshXAnimResult r;
	MeshX* mesh = (MeshX*)meshMan.Get(meshId);
	if (mesh) {
		double now = GetTime();
		mesh->CalcAnimation(0, now, r);
		float wrappedTime = float(now - floor(now));
		auto normToRad = [](float n) { return n * float(M_PI * 2); };
		mesh->Draw(r, translate(0, radius * 1.5f, 0) * q2m(Quat(Vec3(0, 0, 1.0f), normToRad(wrappedTime))));
		mesh->Draw(r, translate(radius * 2.0f, 0, 0) * q2m(Quat(Vec3(0, 1.0f, 0), normToRad(wrappedTime))));
	}
*/
	meshRenderer.Flush();
	luaMan.DrawSprites();
	fontMan.Render();
}

void App::Init()
{
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	afDepthStencilMode(true);

	meshRenderer.Create();
	waterSurface.Init();
	fontMan.Init();
	spriteRenderer.Init();
	stockObjects.Init();
	luaMan.Create();


	LoadMesh("jiji.x");
	PlayBgm("sound/background.mp3");
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
}

void App::Destroy()
{
	luaMan.Destroy();
	stockObjects.Destroy();
	spriteRenderer.Destroy();
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
	inputMan.Update();
	luaMan.Update();
	waterSurface.Update();
	fps.Update();
	fontMan.DrawString(Vec2(20, 40), 20, SPrintf("FPS: %f", fps.Get()));
}
