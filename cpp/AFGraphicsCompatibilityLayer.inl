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
