#if defined(__d3d11_h__) || defined(__d3d12_h__)
#include "AFGraphicsDefinitions.inl"

ComPtr<ID3DBlob> afCompileHLSL(const char* name, const char* entryPoint, const char* target);

typedef unsigned short AFIndex;
#define AFIndexTypeToDevice DXGI_FORMAT_R16_UINT

inline D3D_PRIMITIVE_TOPOLOGY RenderFlagsToPrimitiveTopology(uint32_t flags)
{
	if (flags & AFRS_PRIMITIVE_TRIANGLELIST)
	{
		return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	}
	else if (flags & AFRS_PRIMITIVE_LINELIST)
	{
		return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
	}
	else if (flags & AFRS_PRIMITIVE_PATCHLIST)
	{
		return D3D11_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
	}
	return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}


inline DXGI_FORMAT afTypelessToDSVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_D32_FLOAT;
	}
	return format;
}

inline DXGI_FORMAT afTypelessToSRVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32_TYPELESS:
		return DXGI_FORMAT_R32_FLOAT;
	}
	return format;
}

typedef DXGI_FORMAT AFFormat;
#define AFF_INVALID DXGI_FORMAT_UNKNOWN
#define AFF_R8G8B8A8_UNORM DXGI_FORMAT_R8G8B8A8_UNORM
#define AFF_R8G8B8A8_UNORM_SRGB DXGI_FORMAT_R8G8B8A8_UNORM_SRGB
#define AFF_B8G8R8A8_UNORM DXGI_FORMAT_B8G8R8A8_UNORM
#define AFF_R5G6B5_UINT DXGI_FORMAT_B5G6R5_UNORM
#define AFF_R32G32B32A32_FLOAT DXGI_FORMAT_R32G32B32A32_FLOAT
#define AFF_R16G16B16A16_FLOAT DXGI_FORMAT_R16G16B16A16_FLOAT
#define AFF_DEPTH DXGI_FORMAT_D24_UNORM_S8_UINT
#define AFF_DEPTH_STENCIL DXGI_FORMAT_D24_UNORM_S8_UINT
#define AFF_BC1_UNORM DXGI_FORMAT_BC1_UNORM
#define AFF_BC2_UNORM DXGI_FORMAT_BC2_UNORM
#define AFF_BC3_UNORM DXGI_FORMAT_BC3_UNORM
#define AFF_R32G32_FLOAT DXGI_FORMAT_R32G32_FLOAT
#define AFF_R32G32B32_FLOAT DXGI_FORMAT_R32G32B32_FLOAT
#define AFF_R32G32B32A32_FLOAT DXGI_FORMAT_R32G32B32A32_FLOAT
#define AFF_R32_UINT DXGI_FORMAT_R32_UINT
#define AFF_R8G8B8A8_UINT DXGI_FORMAT_R8G8B8A8_UINT

#endif

void ShowLastWinAPIError();
bool ProcessWindowMessage(HWND hWnd, HACCEL hAccelTable);
bool LoadImageViaGdiPlus(const char* name, IVec2& size, std::vector<uint32_t>& col);

HRESULT _afHandleDXError(const char* file, const char* func, int line, const char* command, HRESULT result);
#define afHandleDXError(command) do{ _afHandleDXError(__FILE__, __FUNCTION__, __LINE__, #command, command); } while(0)
