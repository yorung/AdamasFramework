#include "stdafx.h"

ShaderMan shaderMan;

static GLuint CompileShader(int type, const char *fileName)
{
	GLuint shader = glCreateShader(type);

	void* img = LoadFile(fileName);
	if (!img) {
		aflog("shader file %s not found", fileName);
		return 0;
	}
	glShaderSource(shader, 1, (const char**)&img, NULL);
	glCompileShader(shader);
	free(img);

	int result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		GLchar* buf = new GLchar[len];
		int dummy;
		glGetShaderInfoLog(shader, len, &dummy, buf);
		aflog("result=%d (%s)%s", result, fileName, buf);
		delete buf;
		glDeleteShader(shader);
		shader = 0;
	}
	return shader;
}

static GLuint CreateProgram(const char* name)
{
	char buf[256];
	snprintf(buf, dimof(buf), "shaders/%s.vs", name);
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, buf);
	if (!vertexShader) {
		return 0;
	}
	snprintf(buf, dimof(buf), "shaders/%s.fs", name);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, buf);
	if (!fragmentShader) {
		return 0;
	}
	GLuint program = glCreateProgram();
	glAttachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glAttachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);
	glLinkProgram(program);

	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) {
		GLint len = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		GLchar* buf = new GLchar[len];
		int dummy;
		glGetProgramInfoLog(program, len, &dummy, buf);
		aflog("glLinkProgram failed!=%d (%s)%s", status, name, buf);
		delete buf;
		glDeleteProgram(program);
		program = 0;
	}
	return program;
}

static void SetVertexAttributes(int program, const InputElement elements[], int numElements, int numBuffers, GLuint const *vertexBufferIds, const GLsizei* strides)
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

void ShaderMan::SetVertexBuffers(SMID id, int numBuffers, GLuint const *vertexBufferIds, const GLsizei* strides)
{
	Effect& it = effects[id];
	SetVertexAttributes(id, it.elements, it.numElements, numBuffers, vertexBufferIds, strides);
}

ShaderMan::SMID ShaderMan::Create(const char *name, const InputElement elements[], int numElements)
{
	NameToId::iterator it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}

	Effect effect;
	memset(&effect, 0, sizeof(effect));

	GLuint id = CreateProgram(name);
	effect.elements = elements;
	effect.numElements = numElements;

	effects[id] = effect;
	return nameToId[name] = id;
}

void ShaderMan::Destroy()
{
	for (Effects::iterator it = effects.begin(); it != effects.end(); it++)
	{
		glDeleteProgram(it->first);
	}
	effects.clear();
	nameToId.clear();
}

void ShaderMan::Apply(SMID id)
{
	glUseProgram(id);
}
