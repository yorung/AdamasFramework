#include "stdafx.h"
#include "dev_camera.h"

AFApp* (*AFApp::Generator)();

class AFSampleApp : public AFApp
{
	AFRenderTarget appRenderTarget;
	AFRenderStates copyPSO;
	MeshMan::MMID meshId = MeshMan::INVALID_MMID;
	void Draw(ViewDesc& view);
public:
	virtual void Create() override;
	//	void LoadMesh(const char* fileName);
	virtual void Update() override;
	virtual void Destroy() override;
	virtual void LButtonDown(int x, int y) override
	{
		IVec2 screenSize = systemMisc.GetScreenSize();
		devCamera.LButtonDown(x / (float)screenSize.x, y / (float)screenSize.y);
	}
	virtual void LButtonUp(int x, int y) override
	{
		IVec2 screenSize = systemMisc.GetScreenSize();
		devCamera.LButtonUp(x / (float)screenSize.x, y / (float)screenSize.y);
	}
	virtual void MouseMove(int x, int y) override
	{
		IVec2 screenSize = systemMisc.GetScreenSize();
		devCamera.MouseMove(x / (float)screenSize.x, y / (float)screenSize.y);
	}
};

namespace
{
	AFApp* GenerateSampleApp()
	{
		return new AFSampleApp;
	};

	struct _
	{
		_()
		{
			if (!AFApp::Generator)
			{
				AFApp::Generator = GenerateSampleApp;
			}
		}
	}_;
}

std::string g_type = "mesh";
/*
static float CalcRadius(const Mesh* m)
{
	const Block& b = m->GetRawDatas();
	float maxSq = 0;
	for (auto& it : b.vertices) {
		float sq = lengthSq(it.xyz);
		maxSq = std::max(maxSq, sq);
	}
	return std::sqrt(maxSq);
}*/

void AFSampleApp::Draw(ViewDesc& view)
{
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

	{
		AF_PROFILE_RANGE(AppDrawOffscreen);
		appRenderTarget.BeginRenderToThis();
		moduleManager.Draw3DAll(cmd, appRenderTarget, view);
		luaMan.Draw3D(view);
		meshRenderer.Flush(view);
		skyMan.Draw(cmd, view);
		luaMan.Draw2D(cmd);
		moduleManager.Draw2DAll(cmd, appRenderTarget);
		appRenderTarget.EndRenderToThis();
	}

	{
		AF_PROFILE_RANGE(AppDrawBeginSwapchain);
		afBeginRenderToSwapChain();
	}
	{
		AF_PROFILE_RANGE(AppDrawSetPSO);
		cmd.SetRenderStates(copyPSO);
	}
	{
		AF_PROFILE_RANGE(AppDrawSetVB);
		stockObjects.ApplyFullScreenVertexBuffer(cmd);
	}
	AFTexRef tex;
	{
		AF_PROFILE_RANGE(AppDrawRenderTargetTransition);
		tex = appRenderTarget.GetTexture();
	}
	{
		AF_PROFILE_RANGE(AppDrawTextureBinding);
		cmd.SetTexture(tex, 0);
	}
	{
		AF_PROFILE_RANGE(AppDrawOffscreenBuffer);
		cmd.Draw(4);
	}
	{
		AF_PROFILE_RANGE(AppDrawDrawFontToSwapchain);
		fontMan.Draw(cmd, systemMisc.GetScreenSize());
	}
	{
		AF_PROFILE_RANGE(AppDrawEndSwapchain);
		afEndRenderToSwapChain();
	}
}

static const SamplerType samplers[] =
{
	AFST_POINT_WRAP,
};

void AFSampleApp::Create()
{
#ifdef _MSC_VER
	GoMyDir();
//	SetCurrentDirectoryA("D:\\bitbucket\\Janken\\Program\\pack\\assets");
#endif
#ifdef GL_TRUE
	glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
	glClearDepthf(0);	// for left-handed coordinate
#endif
	debugShapeRenderer.Create();
	meshRenderer.Create();
	fontMan.Create();
	spriteRenderer.Create();
	stockObjects.Create();
	luaMan.Create();
	appRenderTarget.Init(systemMisc.GetScreenSize(), AFF_R8G8B8A8_UNORM, AFF_AUTO_DEPTH_STENCIL);
#if defined(AF_GLES) || defined(AF_VULKAN)
	int numElements = 0;
	const InputElement* elements = stockObjects.GetFullScreenInputElements(numElements);
	copyPSO.Create("glow_copy", numElements, elements, 0, arrayparam(samplers));
#else
	copyPSO.Create("copy_rgba", 0, nullptr, 0, 0, nullptr);
#endif
}

/*
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
*/

void AFSampleApp::Destroy()
{
	deviceMan.Flush();
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
	debugShapeRenderer.Destroy();
	ClearMenu();
	meshId = MeshMan::INVALID_MMID;
}

void AFSampleApp::Update()
{
	ViewDesc view;
	{
		AF_PROFILE_RANGE(Update);
		systemMisc.lastUpdateTime = GetTime();
		inputMan.Update();
		view.screenSize = systemMisc.GetScreenSize();
		view.matView = devCamera.CalcViewMatrix();
		IVec2 scrSize = systemMisc.GetScreenSize();
		float f = 1000;
		float n = 1;
		float aspect = (float)scrSize.x / scrSize.y;
		view.matProj = perspectiveLH(45.0f * (float)M_PI / 180.0f, aspect, n, f);
		moduleManager.UpdateAll();
		luaMan.Update(view);
		fps.Update();
	}
	fontMan.DrawString(Vec2(20, 40), 20, SPrintf("FPS: %f", fps.Get()), 0xffffffff);
	afProfiler.Print();
	Draw(view);
}
