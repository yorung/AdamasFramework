class DebugBoneRenderer
{
	MRID pivotsRenderMeshId;
	Block pivots;
	void CreatePivotMesh();
public:
	DebugBoneRenderer();
	~DebugBoneRenderer();
	void Init();
	void DrawPivots(const Mat mat[BONE_MAX], int num);
	void Destroy();
};

extern DebugBoneRenderer debugBoneRenderer;

void CreateCone(Block& b, const Vec3& v1, const Vec3& v2, BONE_ID boneId, uint32_t color);
