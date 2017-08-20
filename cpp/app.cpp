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
#ifdef GL_TRUE
	afDepthStencilMode(AFRS_DEPTH_ENABLE);	// This needed to clear depth stencil buffer
#endif

	IVec2 scrSize = systemMisc.GetScreenSize();
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
	AFCommandList& cmd = afGetCommandList();

	appRenderTarget.BeginRenderToThis();
	moduleManager.Draw3DAll(cmd, appRenderTarget);
	luaMan.Draw3D();
	meshRenderer.Flush();
	skyMan.Draw(cmd);
	moduleManager.Draw2DAll(cmd, appRenderTarget);
	luaMan.Draw2D(cmd);
	appRenderTarget.EndRenderToThis();

	AFRenderTarget rtDefault;
	rtDefault.InitForDefaultRenderTarget();
	rtDefault.BeginRenderToThis();
	cmd.SetRenderStates(copyPSO);
	stockObjects.ApplyFullScreenVertexBuffer(cmd);
	cmd.SetTexture(appRenderTarget.GetTexture(), 0);
	cmd.Draw(4);
	fontMan.Draw(cmd, systemMisc.GetScreenSize());
	rtDefault.EndRenderToThis();
}

void App::Create()
{
#ifdef _MSC_VER
	GoMyDir();
//	SetCurrentDirectoryA("D:\\bitbucket\\Janken\\Program\\pack\\assets");
#endif
#ifdef GL_TRUE
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClearDepthf(0);	// for left-handed coordinate
#endif
	meshRenderer.Create();
	fontMan.Create();
	spriteRenderer.Create();
	stockObjects.Create();
	luaMan.Create();
	appRenderTarget.Init(systemMisc.GetScreenSize(), AFF_R8G8B8A8_UNORM, AFF_D32_FLOAT_S8_UINT);
#if defined(AF_GLES) || defined(AF_VULKAN)
	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	copyPSO.Create("glow_copy", numElements, elements, 0, 0, nullptr);
#else
	copyPSO.Create("copy_rgba", 0, nullptr, 0, 0, nullptr);
#endif
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

void App::Destroy()
{
#if defined(AF_DX12) || defined(AF_VULKAN)
	deviceMan.Flush();
#endif
	luaMan.Destroy();
	stockObjects.Destroy();
	spriteRenderer.Destroy();
	texMan.Destroy();
	fontMan.Destroy();
	meshRenderer.Destroy();
	meshMan.Destroy();
	skyMan.Destroy();
	glow.Destroy();
	letterBox.Destroy();
	appRenderTarget.Destroy();
	copyPSO.Destroy();
	ClearMenu();
	meshId = MeshMan::INVALID_MMID;
}

void App::Update()
{
	systemMisc.lastUpdateTime = GetTime();
	inputMan.Update();
	matrixMan.Set(MatrixMan::VIEW, devCamera.CalcViewMatrix());
	moduleManager.UpdateAll();
	luaMan.Update();
	fps.Update();
	fontMan.DrawString(Vec2(20, 40), 20, SPrintf("FPS: %f", fps.Get()), 0xffffffff);
	afProfiler.Print();
	Draw();
}
