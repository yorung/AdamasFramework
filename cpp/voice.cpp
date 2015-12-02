#include "stdafx.h"

const void *RiffFindChunk(const void *img, const char *requestChunkName, int *size)
{
	const RiffHeader *riff = (RiffHeader*)img;
	if (*(uint32_t*)riff->type1 != *(uint32_t*)"RIFF" || *(uint32_t*)riff->type2 != *(uint32_t*)"WAVE") {
		return nullptr;
	}

	const void *end = (char*)img + riff->size + 8;
	img = (char*)img + sizeof(RiffHeader);
	while (img < end) {
		const RiffChunk *chunk = (RiffChunk*)img; img = (char*)img + sizeof(RiffChunk);
		if (*(uint32_t*)chunk->type == *(uint32_t*)requestChunkName) {
			if (size) {
				*size = chunk->size;
			}
			return img;
		}
		img = (char*)img + chunk->size;
	}
	return nullptr;
}
