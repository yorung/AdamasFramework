#pragma once

class AFDynamicQuadListVertexBuffer
{
	AFBufferResource ibo;
	uint32_t stride;
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
	void Apply(AFCommandList& cmd, const void* buf, int size)
	{
		assert(size <= vertexBufferSize);
		cmd.SetIndexBuffer(ibo);
		cmd.SetVertexBuffer(size, buf, stride);
	}
	void Destroy()
	{
		afSafeDeleteBuffer(ibo);
	}
};
