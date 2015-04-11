typedef unsigned short AFIndex;

#ifdef __d3d11_h__
#define AFIndexTypeToDevice DXGI_FORMAT_R16_UINT
typedef ID3D11Buffer* IBOID;
typedef ID3D11Buffer* VBOID;
typedef ID3D11Buffer* UBOID;

inline void afSafeDeleteBuffer(AFBufObj& b)
{
	SAFE_RELEASE(b);
}

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi);
IBOID afCreateQuadListIndexBuffer(int numQuads);
VBOID afCreateVertexBuffer(int size, const void* buf);
VBOID afCreateDynamicVertexBuffer(int size);
UBOID afCreateUBO(int size);

void afDrawIndexedTriangleList(IBOID ibo, int count, int start = 0);
#endif
