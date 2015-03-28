#include "stdafx.h"

#ifdef __d3d11_h__
ID3D11Buffer* afCreateIndexBuffer(const AFIndex* indi, int numIndi)
{
	ID3D11Buffer* indexBuffer;
	D3D11_SUBRESOURCE_DATA subresData = { indi, 0, 0 };
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(numIndi * sizeof(AFIndex), D3D11_BIND_INDEX_BUFFER), &subresData, &indexBuffer);
	return indexBuffer;
}

ID3D11Buffer* afCreateDynamicVertexBuffer(int size)
{
	ID3D11Buffer* vbo;
	deviceMan11.GetDevice()->CreateBuffer(&CD3D11_BUFFER_DESC(size, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE), nullptr, &vbo);
	return vbo;
}

void afWriteBuffer(ID3D11Buffer* p, const void* buf, int size)
{
#ifdef _DEBUG
	D3D11_BUFFER_DESC desc;
	p->GetDesc(&desc);
	if (size > (int)desc.ByteWidth) {
		return;
	}
#endif
	D3D11_MAPPED_SUBRESOURCE m;
	HRESULT hr = deviceMan11.GetContext()->Map(p, 0, D3D11_MAP_WRITE_DISCARD, 0, &m);
	memcpy(m.pData, buf, size);
	deviceMan11.GetContext()->Unmap(p, 0);
}

#endif

AFBufObj afCreateQuadListIndexBuffer(int numQuads)
{
	std::vector<AFIndex> indi;
	int numIndi = numQuads * 6;
	indi.resize(numIndi);
	for (int i = 0; i < numIndi; i++)
	{
		static int tbl[] = { 0, 1, 2, 1, 3, 2 };
		int rectIdx = i / 6;
		int vertIdx = i % 6;
		indi[i] = rectIdx * 4 + tbl[vertIdx];
	}
	return afCreateIndexBuffer(&indi[0], numIndi);
}

#ifdef __d3d11_h__
void afDrawIndexedTriangleList(AFBufObj ibo, int count, int start)
{
	deviceMan11.GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	deviceMan11.GetContext()->IASetIndexBuffer(ibo, AFIndexTypeToDevice, 0);
	deviceMan11.GetContext()->DrawIndexed(count, start, 0);
}
#endif
