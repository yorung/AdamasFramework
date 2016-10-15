class AFDynamicQuadListVertexBuffer
{
	IBOID ibo;
	UINT stride;
	int vertexBufferSize;
public:
	~AFDynamicQuadListVertexBuffer() { Destroy(); }
	void Create(int vertexSize_, int nQuad)
	{
		Destroy();
		stride = vertexSize_;
		vertexBufferSize = nQuad * vertexSize_ * 4;
		ibo = afCreateQuadListIndexBuffer(nQuad);
	}
	void Apply()
	{
		afSetIndexBuffer(ibo);
	}
	void Write(const void* buf, int size)
	{
		assert(size <= vertexBufferSize);
		afSetVertexBuffer(size, buf, stride);
	}
	void Destroy()
	{
		afSafeDeleteBuffer(ibo);
	}
};
