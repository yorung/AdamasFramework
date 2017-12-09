#include "stdafx.h"

struct Wave
{
	Vec2 dir;
	float amplitude;
	float waveLength;
};

struct ImmutableCB
{
	float waveSteepness;
	float waveSpeed;
	Wave waves[20];
};

class Ocean : AFModule
{
	VBOID vbo;
	IBOID ibo;
	AFRenderStates renderStates;
	int nIndi = 0;
	const int numGrid = 100;
	const float pitch = 1.0f;
	ImmutableCB cb;
public:
	Ocean();
	~Ocean();
	void Draw3D(AFCommandList& cmd, AFRenderTarget& rt, ViewDesc& viewDesc) override;
};

#define GET_OCEAN \
	Ocean* p = (Ocean*)luaL_checkudata(L, 1, "Ocean");	\
	if (!p) {	\
		return 0;	\
	}

class OceanBinder
{
public:
	OceanBinder()
	{
		GetLuaBindFuncContainer().push_back([](lua_State* L) {
			static luaL_Reg methods[] =
			{
				{ "__gc", [](lua_State* L) { GET_OCEAN p->~Ocean(); return 0; } },
				{ nullptr, nullptr },
			};
			aflBindClass(L, "Ocean", methods, [](lua_State* L) { void* u = lua_newuserdata(L, sizeof(Ocean)); new (u) Ocean(); return 1; });
		});
	}
} static OceanBinder;

struct OceanVert
{
	Vec3 pos;
	Vec3 color;
};

Ocean::~Ocean()
{
	renderStates.Destroy();
	afSafeDeleteBuffer(ibo);
	afSafeDeleteBuffer(vbo);

	assert(!vbo);
	assert(!ibo);
}

Ocean::Ocean()
{
	cb.waveSpeed = 15.f;
	cb.waveSteepness = 0.8f;
	for (int i = 0; i < dimof(cb.waves); i++)
	{
		Wave& w = cb.waves[i];
		float randomRad = (float)(Random() * M_PI * 2 * 0.3f);
		w.dir.x = sinf(randomRad);
		w.dir.y = cosf(randomRad);
		w.amplitude = 0.05f + powf(2.0f, (float)Random() * 2.0f) * 0.38f;
		w.waveLength = powf(2.f, 1.f + (float)Random()) * 10.f;
	}

	const static InputElement layout[] =
	{
		AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
		AF_INPUT_ELEMENT(1, "COLOR", AFF_R32G32B32_FLOAT, 12),
	};
	renderStates.Create("solid", arrayparam(layout), AFRS_DEPTH_ENABLE | AFRS_PRIMITIVE_TRIANGLESTRIP | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL);

	ibo = afCreateTiledPlaneIBO(numGrid, &nIndi);

//	vbo = afCreateVertexBuffer(sizeVertices, &vert[0]);
}

static Vec3 CalcGerstnerWaveOffset(const ImmutableCB& cb, Vec3 location, float time)
{
	Vec3 sum = Vec3(0, 0, 0);
	for (int i = 0; i < dimof(cb.waves); i++)
	{
		const Wave& wave = cb.waves[i];
		float wi = 2 / wave.waveLength;
		float rad = (dot(wave.dir, Vec2(location.x, location.z)) + time * cb.waveSpeed) * wi;
		float sine = std::sin(rad);
		float cosine = std::cos(rad);
		sum.y += sine * wave.amplitude;
		sum.x += wave.dir.x * cosine * cb.waveSteepness / (wi * dimof(cb.waves));
		sum.z += wave.dir.y * cosine * cb.waveSteepness / (wi * dimof(cb.waves));
	}
	return sum;
}

void Ocean::Draw3D(AFCommandList& cmd, AFRenderTarget&, ViewDesc& viewDesc)
{
	cmd.SetRenderStates(renderStates);
	Mat matVP = viewDesc.matView * viewDesc.matProj;
	cmd.SetBuffer(sizeof(Mat), &matVP, 0);

	std::vector<OceanVert> vert;

	float now = (float)GetTime();
	auto add = [&](float x, float z)
	{
		OceanVert v;
		Vec3 pos(x, -10.f, z);
		Vec3 center = pos + CalcGerstnerWaveOffset(cb, pos, now);
		Vec3 offsetXZ = pos + Vec3(1, 0, 1) + CalcGerstnerWaveOffset(cb, pos + Vec3(1, 0, 1), now);
		Vec3 offsetNegX = pos + Vec3(-1, 0, 0) + CalcGerstnerWaveOffset(cb, pos + Vec3(-1, 0, 0), now);
		Vec3 offsetNegZ = pos + Vec3(0, 0, -1) + CalcGerstnerWaveOffset(cb, pos + Vec3(0, 0, -1), now);
		Vec3 normal = normalize(cross(offsetNegZ - offsetXZ, offsetNegX - offsetXZ));

		float illuminance = pow(dot(Vec3(0, 0.707f, 0.707f), normal), 1.f / 2.2f);
		v.pos = center;
		v.color = Vec3(0.3f, 0.5f, 0.9f) * illuminance;

		vert.push_back(v);
	};

	float half = pitch * numGrid / 2;
	for (int z = 0; z <= numGrid; z++)
	{
		for (int x = 0; x <= numGrid; x++)
		{
			add(x * pitch - half, z * pitch - half);
		}
	}

	int sizeVertices = (int)vert.size() * sizeof(OceanVert);

	cmd.SetVertexBuffer(sizeVertices, &vert[0], sizeof(OceanVert));

	cmd.SetIndexBuffer(ibo);
	cmd.DrawIndexed(nIndi);
}
#if 0
void Ocean::Draw3D(AFCommandList& cmd, AFRenderTarget&, ViewDesc& viewDesc)
{
	cmd.SetRenderStates(renderStates);
	Mat matVP = viewDesc.matView * viewDesc.matProj;
	cmd.SetBuffer(sizeof(Mat), &matVP, 0);
	cmd.SetVertexBuffer(vbo, sizeof(OceanVert));
	cmd.SetIndexBuffer(ibo);
	cmd.DrawIndexed(nIndi);
}
#endif