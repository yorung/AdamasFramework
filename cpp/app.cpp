#include "stdafx.h"

App app;

std::string g_type = "mesh";

static float CalcRadius(const Mesh* m)
{
	const Block& b = m->GetRawDatas();
	float maxSq = 0;
	for (auto& it : b.vertices) {
		float sq = lengthSq(it.xyz);
		maxSq = std::max(maxSq, sq);
	}
	return std::sqrt(maxSq);
}

App::App()
{
	meshId = MeshMan::INVALID_MMID;
}

void App::Draw()
{
	afDepthStencilMode(DSM_DEPTH_ENABLE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//	waterSurface.Draw();
//	waterSurfaceClassic.Draw();
/*
	AFRenderTarget rt;
	afSetRenderTarget(rt);
	waterSurface.Draw();
	afSetRenderTarget(nullptr);
	afDrawFullScrenEffect("letterBox", rc);
*/

	ivec2 scrSize = systemMisc.GetScreenSize();
	glViewport(0, 0, scrSize.x, scrSize.y);

	float f = 1000;
	float n = 1;
	float aspect = (float)scrSize.x / scrSize.y;
	Mat proj = perspectiveLH(45.0f * (float)M_PI / 180.0f, aspect, n, f);
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
	luaMan.Draw3D();
	meshRenderer.Flush();
	skyMan.Draw();
	luaMan.Draw2D();
	fontMan.Render();
}

void App::Init()
{
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClearDepthf(0);

#ifdef _MSC_VER
	GoMyDir();
#endif

	meshRenderer.Create();
//	waterSurface.Init();
//	waterSurfaceClassic.Init();
	fontMan.Init();
	spriteRenderer.Init();
	stockObjects.Init();

	luaMan.Create();
}

void App::LoadMesh(const char* fileName)
{
	meshId = meshMan.Create(fileName);
	MeshX* mesh = (MeshX*)meshMan.Get(meshId);
	if (mesh) {
		float radius = CalcRadius(mesh);
		float scale = std::max(0.00001f, radius);
		devCamera.SetDistance(scale * 3);
	}
	g_type = "mesh";
}

void App::OnTap(float x, float y)
{
//	waterSurfaceClassic.CreateRipple(Vec2(x, y));
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
	glow.Destroy();
	skyMan.Destroy();
	ClearMenu();
	meshId = MeshMan::INVALID_MMID;
}

void App::Update()
{
	inputMan.Update();
	matrixMan.Set(MatrixMan::VIEW, devCamera.CalcViewMatrix());
	luaMan.Update();
	waterSurface.Update();
	fps.Update();
	fontMan.DrawString(Vec2(20, 40), 20, SPrintf("FPS: %f", fps.Get()));
}
