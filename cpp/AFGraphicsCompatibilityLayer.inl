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

#ifndef AF_DX11
typedef SRVID AFTexRef;
#endif
