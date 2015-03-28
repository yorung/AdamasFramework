#include "stdafx.h"

GLuint afCreateIndexBuffer(const AFIndex* indi, int numIndi)
{
	GLuint ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndi * sizeof(AFIndex), &indi[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}

AFBufObj afCreateVertexBuffer(int size, const void* buf)
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}

AFBufObj afCreateDynamicVertexBuffer(int size)
{
	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}

void afWriteBuffer(GLuint bufName, const void* buf, int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, bufName);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, buf);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint afCreateDynamicTexture(int w, int h, AFDTFormat format)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	switch (format) {
	case AFDT_R8G8B8A8_UINT:
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		break;
	case AFDT_R5G6B5_UINT:
		glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, nullptr);
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void afDrawIndexedTriangleList(AFBufObj ibo, int count, int start)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, count, AFIndexTypeToDevice, (void*)(start));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
