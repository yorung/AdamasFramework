class AFCommandList
{
	AFRenderStates* currentRS = nullptr;
public:
	void SetRenderStates(AFRenderStates& rs)
	{
		rs.Apply();
		currentRS = &rs;
	}
	void SetTexture(SRVID texId, int descritorSetIndex)
	{
#ifdef AF_VULKAN
		afBindTexture(currentRS->GetPipelineLayout(), texId, descritorSetIndex);
#else
		afBindTexture(texId, descritorSetIndex);
#endif
	}
	void SetBuffer(int size, const void* buf, int descritorSetIndex)
	{
#ifdef AF_VULKAN
		afBindBuffer(currentRS->GetPipelineLayout(), size, buf, descritorSetIndex);
#else
		afBindBuffer(size, buf, descritorSetIndex);
#endif
	}
#ifndef AF_VULKAN
	void SetBuffer(UBOID uniformBuffer, int descriptorSetIndex)
	{
		afBindBuffer(uniformBuffer, descriptorSetIndex);
	}
#endif
#ifndef AF_GLES31
	void SetVertexBuffer(int size, const void* buf, int stride)
	{
		afSetVertexBuffer(size, buf, stride);
	}
#endif
	void SetVertexBuffer(VBOID vertexBuffer, int stride)
	{
		afSetVertexBuffer(vertexBuffer, stride);
	}
	void SetIndexBuffer(IBOID indexBuffer)
	{
		afSetIndexBuffer(indexBuffer);
	}
	void Draw(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDraw(numVertices, start, instanceCount);
	}
	void DrawIndexed(int numVertices, int start = 0, int instanceCount = 1)
	{
		afDrawIndexed(numVertices, start, instanceCount);
	}
};

inline AFCommandList& afGetCommandList()
{
	static AFCommandList commandList;
	return commandList;
}
