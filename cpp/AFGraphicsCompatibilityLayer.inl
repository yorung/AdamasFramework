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
#ifdef AF_GLES31
		const InputElement* elements;
		int numElements;
		currentRS->GetInputElements(elements, numElements);
		afSetVertexAttributes(elements, numElements, 1, &vertexBuffer, &stride);
#else
		afSetVertexBuffer(vertexBuffer, stride);
#endif
	}
	void SetIndexBuffer(IBOID indexBuffer)
	{
		afSetIndexBuffer(indexBuffer);
	}
	void Draw(int numVertices, int start = 0, int instanceCount = 1)
	{
#ifdef AF_GLES31
		afDraw(currentRS->GetPrimitiveTopology(), numVertices, start, instanceCount);
#else
		afDraw(numVertices, start, instanceCount);
#endif
	}
	void DrawIndexed(int numVertices, int start = 0, int instanceCount = 1)
	{
#ifdef AF_GLES31
		afDrawIndexed(currentRS->GetPrimitiveTopology(), numVertices, start, instanceCount);
#else
		afDrawIndexed(numVertices, start, instanceCount);
#endif
	}
};

inline AFCommandList& afGetCommandList()
{
	static AFCommandList commandList;
	return commandList;
}


#ifdef AF_VULKAN
#define AF_INPUT_ELEMENT(index,name,format,offset) CInputElement(index, format, offset)
#else
#define AF_INPUT_ELEMENT(index,name,format,offset) CInputElement(name, format, offset)
#endif