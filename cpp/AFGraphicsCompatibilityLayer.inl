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

#if defined(AF_VULKAN) && defined(_MSC_VER)	// Desktop Vulkan should use D32/S8 depth stencil for Radeon devices
#define AFF_AUTO_DEPTH_STENCIL AFF_D32_FLOAT_S8_UINT
#else
#define AFF_AUTO_DEPTH_STENCIL AFF_D24_UNORM_S8_UINT
#endif

inline AFFormat afRenderFlagsToDSFormat(uint32_t flags)
{
	if (flags & AFRS_AUTO_DEPTH_STENCIL)
	{
		return AFF_AUTO_DEPTH_STENCIL;
	}
	if (flags & AFRS_DEPTH_STENCIL_D32_FLOAT)
	{
		return AFF_D32_FLOAT;
	}
	return AFF_INVALID;
}

inline AFFormat afRenderFlagsToRTFormat(uint32_t flags)
{
	if (flags & AFRS_OFFSCREEN_RENDER_TARGET_R8G8B8A8_UNORM)
	{
		return AFF_R8G8B8A8_UNORM;
	}
	if (flags & AFRS_OFFSCREEN_RENDER_TARGET_R16G16B16A16_FLOAT)
	{
		return AFF_R16G16B16A16_FLOAT;
	}
	if (flags & AFRS_OFFSCREEN_RENDER_TARGET_R32_FLOAT)
	{
		return AFF_R32_FLOAT;
	}
	assert(0);
	return AFF_INVALID;
}
