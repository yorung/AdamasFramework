#include "stdafx.h"

DebugBoneRenderer debugBoneRenderer;
DebugShapeRenderer debugShapeRenderer;

static void InitVertex(MeshVertex& v, uint32_t color, BONE_ID boneId)
{
	assert(boneId <= std::numeric_limits<uint8_t>::max());
	v.normal.x = 1;
	v.normal.y = 0;
	v.normal.z = 0;
	v.xyz.x = v.xyz.y = v.xyz.z = 0;
	v.color = color;
	v.uv.x = v.uv.y = 0;
	v.blendIndices.x = v.blendIndices.y = v.blendIndices.z = v.blendIndices.w = (uint8_t)boneId;
	v.blendWeights.x = v.blendWeights.y = v.blendWeights.z = 0;
}

void CreateCone(Block& b, const Vec3& v1, const Vec3& v2, BONE_ID boneId, uint32_t color)
{
	float radius = 0.15f;
	Vec3 boneDir = v2 - v1;
	Vec3 vRot0 = cross(boneDir, Vec3(0, 0, radius));
	if (length(vRot0) < 0.001f) {
		vRot0 = cross(boneDir, Vec3(0, radius, 0));
	}
	Vec3 vRot90 = cross(vRot0, normalize(boneDir));
	Vec3 vRotLast = v1 + vRot0;
	static const int div = 10;
	for (int j = 0; j < div; j++) {
		MeshVertex vert[3];
		for (auto& it : vert) {
			InitVertex(it, color, boneId);
		}
		float rad = ((float)M_PI * 2) / div * (j + 1);
		Vec3 vRot = v1 + vRot0 * std::cos(rad) + vRot90 * std::sin(rad);
		vert[0].xyz = vRotLast;
		vert[1].xyz = v2;
		vert[2].xyz = vRot;
		Vec3 normal = cross(vRotLast - v2, v2 - vRot);
		for (int i = 0; i < 3; i++) {
			vert[i].normal = normal;
			b.vertices.push_back(vert[i]);
			b.indices.push_back((AFIndex)b.indices.size());
		}
		vRotLast = vRot;
	}
}

void DebugBoneRenderer::CreatePivotMesh()
{
	for (BONE_ID i = 0; (unsigned)i < BONE_MAX; i++)	{
		float len = 12.0f;
		CreateCone(pivots, Vec3(), Vec3(len, 0, 0), i, 0xff0000ff);
		CreateCone(pivots, Vec3(), Vec3(0, len, 0), i, 0xff00ff00);
		CreateCone(pivots, Vec3(), Vec3(0, 0, len), i, 0xffff0000);
	}

	pivotsRenderMeshId = meshRenderer.CreateRenderMesh(pivots);

	Material mat;
	mat.faceColor.x = 0.6f;
	mat.faceColor.y = 0.6f;
	mat.faceColor.z = 0.6f;
	mat.faceColor.w = 1.0f;
	mat.power = 1.0f;
	mat.specular.x = 1.0f;
	mat.specular.y = 1.0f;
	mat.specular.z = 1.0f;
	mat.emissive.x = 0.4f;
	mat.emissive.y = 0.4f;
	mat.emissive.z = 0.4f;
	mat.texture = texMan.CreateWhiteTexture();

	MaterialMap map;
	map.materialId = meshRenderer.CreateMaterial(mat);
	map.faceStartIndex = 0;
	map.faces = (int)pivots.indices.size() / 3;
	pivots.materialMaps.push_back(map);
}

DebugBoneRenderer::DebugBoneRenderer()
{
}

DebugBoneRenderer::~DebugBoneRenderer()
{
	Destroy();
}

void DebugBoneRenderer::Init()
{
	CreatePivotMesh();
}

void DebugBoneRenderer::DrawPivots(const ViewDesc& view, const Mat mat[BONE_MAX], int num)
{
	Mat mat2[BONE_MAX];
	for (int i = 0; i < BONE_MAX; i++) {
		mat2[i] = i < num ? mat[i] : Mat();
	}

	meshRenderer.DrawRenderMesh(view, pivotsRenderMeshId, Mat(), mat2, BONE_MAX);

	for (int i = 0; i < BONE_MAX; i++) {
		mat2[i] = orthogonalize(mat2[i]);
	}
	meshRenderer.DrawRenderMesh(view, pivotsRenderMeshId, Mat(), mat2, BONE_MAX);
}

void DebugBoneRenderer::Destroy()
{
	meshRenderer.SafeDestroyRenderMesh(pivotsRenderMeshId);
}

void DebugShapeRenderer::DrawSolidPolygons(Mat& mat, int nVertices, DebugSolidVertex vertices[])
{
	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(polygonRenderStates);
	cmd.SetBuffer(sizeof(Mat), &mat, 0);
	cmd.SetVertexBuffer(sizeof(DebugSolidVertex) * nVertices, vertices, sizeof(DebugSolidVertex));
	cmd.Draw(nVertices);
}

void DebugShapeRenderer::DrawSolidLines(Mat& mat, int nVertices, DebugSolidVertex vertices[])
{
	AFCommandList& cmd = afGetCommandList();
	cmd.SetRenderStates(lineRenderStates);
	cmd.SetBuffer(sizeof(Mat), &mat, 0);
	cmd.SetVertexBuffer(sizeof(DebugSolidVertex) * nVertices, vertices, sizeof(DebugSolidVertex));
	cmd.Draw(nVertices);
}

void DebugShapeRenderer::Create()
{
	const static InputElement elements[] =
	{
		AF_INPUT_ELEMENT(0, "POSITION", AFF_R32G32B32_FLOAT, 0),
		AF_INPUT_ELEMENT(1, "COLOR", AFF_R32G32B32_FLOAT, 12),
	};
	polygonRenderStates.Create("solid", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL);
	lineRenderStates.Create("solid", arrayparam(elements), AFRS_DEPTH_ENABLE | AFRS_PRIMITIVE_LINELIST | AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM | AFRS_AUTO_DEPTH_STENCIL);
}

void DebugShapeRenderer::Destroy()
{
	polygonRenderStates.Destroy();
	lineRenderStates.Destroy();
}
