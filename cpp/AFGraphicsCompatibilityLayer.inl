#ifdef AF_VULKAN
inline void afBindBuffer(AFRenderStates& rs, int size, const void* buf, int descritorSetIndex)
{
	afBindBuffer(rs.GetPipelineLayout(), size, buf, descritorSetIndex);
}

inline void afBindTexture(AFRenderStates& rs, const TextureContext& textureContext, int descritorSetIndex)
{
	afBindTexture(rs.GetPipelineLayout(), textureContext, descritorSetIndex);
}
#else
inline void afBindBuffer(AFRenderStates& rs, int size, const void* buf, int descritorSetIndex)
{
	afBindBuffer(size, buf, descritorSetIndex);
}

inline void afBindTexture(AFRenderStates& rs, SRVID texId, int descritorSetIndex)
{
	afBindTexture(texId, descritorSetIndex);
}
#endif
