#include "stdafx.h"
#include "bvh.h"
#include "debug_renderer.h"
#include "tex_man.h"

extern std::string g_type;

static const char *bvhPerfume[] =
{
	"RightWrist",
	"LeftWrist",
	"RightElbow",
	"LeftElbow",
	"RightShoulder",
	"LeftShoulder",
	"RightCollar",
	"LeftCollar",
	"Head",
	"Neck",
	"Chest4",
	"Chest3",
	"Chest2",
	"Chest",
	"Hips",
	"RightHip",
	"LeftHip",
	"RightKnee",
	"LeftKnee",
	"RightAnkle",
	"LeftAnkle",
	"RightToe",
	"LeftToe",
};

static const char *bvhMocap[] =
{
	"RightWrist",
	"LeftWrist",
	"RightElbow",
	"LeftElbow",
	"RightShoulder",
	"LeftShoulder",
	"RightCollar",
	"LeftCollar",
	"Head",
	"Neck",
	"Chest2",
	"Chest2",
	"Chest",
	"Chest",
	"Hips",
	"RightHip",
	"LeftHip",
	"RightKnee",
	"LeftKnee",
	"RightAnkle",
	"LeftAnkle",
	"End of RightAnkle",
	"End of LeftAnkle",
};

static const char *bvhCmu[] =
{
	"RightHand",
	"LeftHand",
	"RightForeArm",
	"LeftForeArm",
	"RightArm",
	"LeftArm",
	"RightShoulder",
	"LeftShoulder",
	"Head",
	"Neck",
	"Spine1",
	"Spine1",
	"Spine",
	"LowerBack",
	"Hips",
	"RightUpLeg",
	"LeftUpLeg",
	"RightLeg",
	"LeftLeg",
	"RightFoot",
	"LeftFoot",
	"RightToeBase",
	"LeftToeBase",
};

static const char *bvhMixamo[] =
{
	"RightHand",
	"LeftHand",
	"RightForeArm",
	"LeftForeArm",
	"RightArm",
	"LeftArm",
	"RightShoulder",
	"LeftShoulder",
	"Head",
	"Neck",
	"Spine2",
	"Spine1",
	"Spine1",
	"Spine",
	"Hips",
	"RightUpLeg",
	"LeftUpLeg",
	"RightLeg",
	"LeftLeg",
	"RightFoot",
	"LeftFoot",
	"RightToeBase",
	"LeftToeBase",
};

static const char **bvhPresets[] = {bvhPerfume, bvhMocap, bvhCmu, bvhMixamo};

static void _enterBrace(char*& p)
{
	if (!p) {
		return;
	}

	p = strchr(p, '{');
	if (p) {
		p++;
	}
}

static char* _leaveBrace(char* p)
{
	if (!p) {
		return nullptr;
	}

	int depth = 0;
	while (p && *p) {
		if (*p == '{') {
			depth++;
		} else if (*p == '}') {
			if (--depth < 0) {
				p++;
				return p;
			}
		}
		p++;
	}
	return nullptr;
}

static double _getD(char*& p)
{
	if (!p)	{
		return 0;
	}

	while (true) {
		if (strchr("0123456789-.", *p))
		{
			break;
		}
		p++;
	}

	double result = atof(p);

	while (*p) {
		if (!strchr("0123456789-.eE", *p)) {
			break;
		}
		p++;
	}
	return result;
}

static int _getI(char*& p)
{
	return (int)_getD(p);
}

static float _getF(char*& p)
{
	return (float)_getD(p);
}

static std::string _getToken(char*& p)
{
	while(*p && !isalpha(*p)) {
		if (*p == '{' || *p == '}') {
			return "";
		}
		p++;
	}

	std::string r;
	while (*p && (isalnum(*p) || *p == '_' || *p == ':')) {
		r = r + *p++;
	}

	return r;
}

static char* _searchNoNameChildTag(char* from)
{
	char* p = from;

	if (!p) {
		return nullptr;
	}

	int depth = 0;
	while (p && *p) {
		if (*p == '{') {
			return p + 1;
		} else if (*p == '}') {
			if (--depth < 0) {
				return nullptr;
			}
		} else if (isalpha(*p)) {
			_enterBrace(p);
			depth++;
			continue;
		}
		p++;
	}
	return nullptr;
}

static char* _searchChildTag(char* from, const char *tag, std::string* name = nullptr)
{
	if (*tag == '\0') {
		return _searchNoNameChildTag(from);
	}

	char* p = from;

	if (!p)
		return nullptr;

	size_t tagLen = strlen(tag);

	int depth = 0;
	while (p && *p) {
		if (*p == '#') {
			p = strchr(p, '\n');
			if (!p) {
				return nullptr;
			}
		} else if (*p == '{') {
			depth++;
		} else if (*p == '}') {
			if (--depth < 0) {
				return nullptr;
			}
		} else if (depth == 0 && isalpha(*p)) {

			if (!strncmp(p, tag, tagLen) && !isalpha(p[tagLen])) {
				p += tagLen + 1;
				if (name) {
					*name = _getToken(p);
				}
				_enterBrace(p);
				return p;
			}
			_getToken(p);
			continue;
		}
		p++;
	}
	return nullptr;
}

int Bvh::GetDepth(BONE_ID id)
{
	int depth = 0;
	BvhFrame* f = &m_frames[id];
	for (; f->parentId >= 0; depth++) {
		f = &m_frames[f->parentId];
	}
	return depth;
}

void Bvh::CreateBoneTypeToIdTbl()
{
	for (int i = 0; i < BT_MAX; i++) {
		boneTypeToIdTbl[i] = -1;
	}
	for (auto& it : bvhPresets) {
		int cnt = 0;
		for (int i = 0; i < BT_MAX; i++) {
			for (BONE_ID id = 0; id < (BONE_ID)m_frames.size(); id++) {
				const BvhFrame& f = m_frames[id];
				if (!strcmp(f.name, it[i])) {
					cnt++;
					boneTypeToIdTbl[i] = id;
				}
			}
		}
		if (cnt == BT_MAX) {
			return;
		}
	}
}

Bvh::Bvh(const char *fileName)
{
	LoadSub(fileName);

	CalcCombinedOffsets();
	for (BONE_ID i = 0; i < (BONE_ID)m_frames.size(); i++) {
		CalcBoneOffsetMatrix(i);
	}
	CreateBoneMesh();

	for (BONE_ID i = 0; i < (BONE_ID)m_frames.size(); i++) {
		if (!memcmp(m_frames[i].name, "mixamorig:", 10)) {
			memmove(m_frames[i].name, m_frames[i].name + 10, strlen(m_frames[i].name + 10) + 1);
		}
	}
	CreateBoneTypeToIdTbl();
}

void Bvh::CreateBoneMesh()
{
	m_block.Clear();

	for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++)	{
		BvhFrame& f2 = m_frames[i];
		BONE_ID pId = f2.parentId;
		if (pId < 0) {
			continue;
		}
		int depth = GetDepth(pId);
		BvhFrame& f1 = m_frames[pId];
		Vec3 v1 = f1.offsetCombined;
		Vec3 v2 = f2.offsetCombined;

		static const uint32_t depthToColor[] = {
			0xffffffff,
			0xffffff00,
			0xffff00ff,
			0xffff0000,
			0xff00ffff,
			0xff00ff00,
			0xff0000ff,
			0xff000000,
		};
		CreateCone(m_block, v1, v2, pId, depthToColor[depth % dimof(depthToColor)]);
	}

	renderMeshId = meshRenderer.CreateRenderMesh(m_block);

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
	map.faces = (int)m_block.indices.size() / 3;
	m_block.materialMaps.push_back(map);
}

BONE_ID Bvh::_getFrameIdByName(const char* name)
{
	for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++) {
		if (!strcmp(m_frames[i].name, name)) {
			return i;
		}
	}
	BvhFrame f;
	assert(strlen(name) < sizeof(f.name));
	strcpy_s(f.name, sizeof(f.name), name);
	f.parentId = -1;
	f.childId = -1;
	f.siblingId = -1;
	f.rotIndices.x = -1;
	f.rotIndices.y = -1;
	f.rotIndices.z = -1;
	f.posIndices.x = -1;
	f.posIndices.y = -1;
	f.posIndices.z = -1;
	m_frames.push_back(f);
	return (int)m_frames.size() - 1;
}

void Bvh::_linkFrame(BONE_ID parentFrameId, BONE_ID childFrameId)
{
	BvhFrame* frameParent = &m_frames[parentFrameId];
	BvhFrame* frameChild = &m_frames[childFrameId];

	frameChild->parentId = parentFrameId;
	frameChild->offsetCombined = frameChild->offset * frameParent->offsetCombined;

	if (frameParent->childId < 0) {
		frameParent->childId = childFrameId;
	} else {
		BONE_ID siblingId = frameParent->childId;
		while (m_frames[siblingId].siblingId >= 0) {
			siblingId = m_frames[siblingId].siblingId;
		}
		m_frames[siblingId].siblingId = childFrameId;
	}
}

void Bvh::CalcBoneOffsetMatrix(BONE_ID frameId)
{
	BvhFrame& frame = m_frames[frameId];
	frame.boneOffsetMatrix = v2m(-frame.offsetCombined);
}

void Bvh::ParseFrame(const char* frameStr, char* p, BONE_ID parentFrameId)
{
	while(p) {
		std::string name;
		char* child = _searchChildTag(p, frameStr, &name);
		if (child) {
			if (name == "Site") {
				name = std::string("End of ") + m_frames[parentFrameId].name;
			}
			BONE_ID frameId = _getFrameIdByName(name.c_str());
			BvhFrame& frame = m_frames[frameId];

			_getToken(child);	// "OFFSET"
			frame.offset.x = _getF(child);
			frame.offset.y = _getF(child);
			frame.offset.z = -_getF(child);
			frame.offsetCombined = Vec3();

			if (parentFrameId >= 0) {
				_linkFrame(parentFrameId, frameId);
			}

			CalcBoneOffsetMatrix(frameId);

			if ("CHANNELS" == _getToken(child)) {
				int nChannels = _getI(child);
				for (int i = 0; i < nChannels; i++) {
					std::string t = _getToken(child);
					if (t == "Xposition") {
						frame.posIndices.x = channels++;
					} else if (t == "Yposition") {
						frame.posIndices.y = channels++;
					} else if (t == "Zposition") {
						frame.posIndices.z = channels++;
					} else if (t == "Xrotation") {
						frame.rotIndices.x = channels++;
					} else if (t == "Yrotation") {
						frame.rotIndices.y = channels++;
					} else if (t == "Zrotation") {
						frame.rotIndices.z = channels++;
					}
				}
			}

			ParseFrame("JOINT", child, frameId);
			ParseFrame("End", child, frameId);
		}
		p = _leaveBrace(child);
	}
}

void Bvh::DumpFrames(BONE_ID frameId, int depth) const
{
	const BvhFrame& f = m_frames[frameId];
	for (int i = 0; i < depth; i++) {
		printf(" ");
	}
	printf("%s(%d) p=%d s=%d c=%d ", f.name, frameId, f.parentId, f.siblingId, f.childId);
	const Vec3& v = f.offset;
	const Vec3& v2 = f.offsetCombined;
	printf("(%3.3f,%3.3f,%3.3f) (%3.3f,%3.3f,%3.3f)", v.x, v.y, v.z, v2.x, v2.y, v2.z);
	printf("\n");
	if (f.siblingId >= 0) {
		DumpFrames(f.siblingId, depth);
	}
	if (f.childId >= 0) {
		DumpFrames(f.childId, depth + 1);
	}
}

void Bvh::ParseMotion(const char *p)
{
	char *m = (char*)strstr(p, "MOTION");
	motionFrames = _getI(m);
	frameTime = _getF(m);
	while (m && *m) {
		rawMotion.push_back(_getF(m));
	}
}

void Bvh::PreCalculateMotion()
{
	motion.poses.resize(motionFrames);
	for (int i = 0; i < motionFrames; i++) {
		const float* mot = &rawMotion[i * channels];
		Pose& pose = motion.poses[i];

		for (auto& it : m_frames) {
			Quat q[3];
			int idxMin = std::min((uint32_t)it.rotIndices.x, std::min((uint32_t)it.rotIndices.y, (uint32_t)it.rotIndices.z));
			if (it.rotIndices.x >= 0) {
				q[it.rotIndices.x - idxMin] = Quat(Vec3(1,0,0), -mot[it.rotIndices.x] * (float)M_PI / 180);
			}
			if (it.rotIndices.y >= 0) {
				q[it.rotIndices.y - idxMin] = Quat(Vec3(0,1,0), -mot[it.rotIndices.y] * (float)M_PI / 180);
			}
			if (it.rotIndices.z >= 0) {
				q[it.rotIndices.z - idxMin] = Quat(Vec3(0,0,1), mot[it.rotIndices.z] * (float)M_PI / 180);
			}
			pose.quats.push_back(q[2] * q[1] * q[0]);
		}
	}
}

void Bvh::LoadSub(const char *fileName)
{
	void *img = LoadFile(fileName);
	if (!img) {
		return;
	}

	char* body = (char*)img;

	channels = 0;
	ParseFrame("ROOT", body, -1);
	ParseMotion(body);
	PreCalculateMotion();
	free(img);

	printf("===============DumpFrames begin\n");
	DumpFrames(0, 0);
	printf("===============DumpFrames end\n");

}

Bvh::~Bvh()
{
	meshRenderer.SafeDestroyRenderMesh(renderMeshId);
}

void Bvh::CalcFrameMatrices()
{
	for (auto& f : m_frames) {
		f.result = f.frameTransformMatrix * (f.parentId >= 0 ? m_frames[f.parentId].result : Mat());
	}
}

void Bvh::CalcCombinedOffsets()
{
	for (auto& f : m_frames) {
		f.offsetCombined = f.offset + (f.parentId >= 0 ? m_frames[f.parentId].offsetCombined : Vec3());
	}
}

void Bvh::CalcAnimation(double time)
{
	int frame = (int)(time / frameTime);
	frame %= motionFrames;

	const float* mot = &rawMotion[frame * channels];
	const Pose& pose = motion.poses[frame];

	for (BONE_ID i = 0; i < (BONE_ID)m_frames.size(); i++) {
		auto& it = m_frames[i];
		Vec3 translate;
		Quat q = pose.quats[i];
		if (it.posIndices.x >= 0) {
			translate = Vec3(mot[it.posIndices.x], mot[it.posIndices.y], -mot[it.posIndices.z]);
		} else {
			translate = it.offset;
		}
		
		it.frameTransformMatrix = q2m(q) * v2m(translate);
	}
	CalcFrameMatrices();
}

void Bvh::ResetAnim()
{
	for (auto& it : m_frames) {
		it.frameTransformMatrix = v2m(it.offset);
	}
	CalcFrameMatrices();
}

void Bvh::Draw(const ViewDesc& view, int animId, double time)
{
	if (!m_block.indices.size()) {
		return;
	}

	Mat BoneMatrices[BONE_MAX];
	assert(m_frames.size() <= dimof(BoneMatrices));

	if (animId != 0) {
		for (auto& it : m_frames) {
			it.frameTransformMatrix = v2m(it.offset);
		}
		CalcFrameMatrices();
	} else {
		CalcAnimation(time);
	}

	if (g_type == "pivot") {
		for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++)	{
			BvhFrame& f = m_frames[i];
			BoneMatrices[i] = f.result;
		}
		debugBoneRenderer.DrawPivots(view, BoneMatrices, (int)m_frames.size());
	} else {
		for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++)	{
			BvhFrame& f = m_frames[i];
			BoneMatrices[i] = f.boneOffsetMatrix * f.result;
		}
		meshRenderer.DrawRenderMesh(view, renderMeshId, Mat(), BoneMatrices, (int)m_frames.size());
	}
}

void Bvh::GetRotAnim(Quat quats[BONE_MAX], double time) const
{
	int frame = (int)(time / frameTime);
	frame %= motionFrames;

	const Pose& pose = motion.poses[frame];

	for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++)	{
		quats[i] = pose.quats[i];
	}
}

BONE_ID Bvh::BoneNameToId(const char* name) const
{
	for (BONE_ID i = 0; (unsigned)i < m_frames.size(); i++)	{
		const BvhFrame& f = m_frames[i];
		if (!strcmp(f.name, name)) {
			return i;
		}
	}
	return -1;
}

BONE_ID Bvh::BoneTypeToId(BoneType type) const
{
	return boneTypeToIdTbl[type];
}
