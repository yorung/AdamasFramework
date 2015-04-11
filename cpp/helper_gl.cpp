#include "stdafx.h"

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi)
{
	IBOID ibo;
	glGenBuffers(1, &ibo.x);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndi * sizeof(AFIndex), &indi[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}

VBOID afCreateVertexBuffer(int size, const void* buf)
{
	VBOID vbo;
	glGenBuffers(1, &vbo.x);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}

VBOID afCreateDynamicVertexBuffer(int size)
{
	VBOID vbo;
	glGenBuffers(1, &vbo.x);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}

SSBOID afCreateSSBO(int size)
{
	SSBOID name;
	glGenBuffers(1, &name.x);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, name);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return name;
}

UBOID afCreateUBO(int size)
{
	UBOID name;
	glGenBuffers(1, &name.x);
	glBindBuffer(GL_UNIFORM_BUFFER, name);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return name;
}

#if 0 // without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding)
{
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, name), samplerBinding);
}

void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding)
{
	glShaderStorageBlockBinding(program, glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name), storageBlockBinding);
}

void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding)
{
	glUniformBlockBinding(program, glGetUniformBlockIndex(program, name), uniformBlockBinding);
}
#endif
void afBindBufferToBindingPoint(SSBOID ssbo, GLuint storageBlockBinding)
{
	GLint prev;
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &prev);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, storageBlockBinding, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, prev);
}

void afBindBufferToBindingPoint(UBOID ubo, GLuint uniformBlockBinding)
{
	GLint prev;
	glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &prev);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, prev);
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

void afDrawIndexedTriangleList(IBOID ibo, int count, int start)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glDrawElements(GL_TRIANGLES, count, AFIndexTypeToDevice, (void*)(start));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides)
{
	for (int i = 0; i < numElements; i++) {
		const InputElement& d = elements[i];
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIds[d.inputSlot]);
		GLint h = glGetAttribLocation(program, d.name);
		if (h == -1) {
			continue;
		}
		glEnableVertexAttribArray(h);
		switch (d.format) {
		case SF_R32_FLOAT:
		case SF_R32G32_FLOAT:
		case SF_R32G32B32_FLOAT:
		case SF_R32G32B32A32_FLOAT:
			glVertexAttribPointer(h, d.format - SF_R32_FLOAT + 1, GL_FLOAT, GL_FALSE, strides[d.inputSlot], (void*)d.offset);
			break;
		case SF_R8_UNORM:
		case SF_R8G8_UNORM:
		case SF_R8G8B8_UNORM:
		case SF_R8G8B8A8_UNORM:
			glVertexAttribPointer(h, d.format - SF_R8_UNORM + 1, GL_UNSIGNED_BYTE, GL_TRUE, strides[d.inputSlot], (void*)d.offset);
			break;
		case SF_R8_UINT:
		case SF_R8G8_UINT:
		case SF_R8G8B8_UINT:
		case SF_R8G8B8A8_UINT:
			//			glVertexAttribPointer(h, d.format - SF_R8_UINT + 1, GL_UNSIGNED_BYTE, GL_FALSE, strides[d.inputSlot], (void*)d.offset);
			glVertexAttribIPointer(h, d.format - SF_R8_UINT + 1, GL_UNSIGNED_BYTE, strides[d.inputSlot], (void*)d.offset);
			break;
		case SF_R16_UINT:
		case SF_R16G16_UINT:
		case SF_R16G16B16_UINT:
		case SF_R16G16B16A16_UINT:
			//			glVertexAttribPointer(h, d.format - SF_R16_UINT + 1, GL_UNSIGNED_SHORT, GL_FALSE, strides[d.inputSlot], (void*)d.offset);
			glVertexAttribIPointer(h, d.format - SF_R16_UINT + 1, GL_UNSIGNED_SHORT, strides[d.inputSlot], (void*)d.offset);
			break;
		}
		if (d.perInstance) {
			glVertexAttribDivisor(h, 1);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint afCreateVAO(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides, IBOID ibo)
{
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	afSetVertexAttributes(program, elements, numElements, numBuffers, vertexBufferIds, strides);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);
	return vao;
}

IBOID afCreateQuadListIndexBuffer(int numQuads)
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
