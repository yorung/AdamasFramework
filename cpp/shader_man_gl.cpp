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
	} else {
		aflog("CompileShader(%s) succeess. id=%d\n", fileName, shader);
	}
	return shader;
}

static GLuint CreateProgram(const char* name, const InputElement elements[], int numElements)
{
	char buf[256];
	snprintf(buf, dimof(buf), "glsl/%s.vert", name);
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, buf);
	if (!vertexShader) {
		return 0;
	}
	snprintf(buf, dimof(buf), "glsl/%s.frag", name);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, buf);
	if (!fragmentShader) {
		return 0;
	}
	GLuint program = glCreateProgram();
	for (int i = 0; i < numElements; i++) {
		glBindAttribLocation(program, i, elements[i].name);
	}

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
	} else {
		aflog("CreateProgram(%s) succeess. id=%d\n", name, program);
	}
	return program;
}

ShaderMan::SMID ShaderMan::Create(const char *name, const InputElement elements[], int numElements, BlendMode blendMode, DepthStencilMode depthStencilMode, CullMode cullMode)
{
	NameToId::iterator it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	SMID id = nameToId[name] = CreateProgram(name, elements, numElements);
	if (id != 0) {
		Effect e;
		e.blendMode = blendMode;
		e.depthStencilMode = depthStencilMode;
		e.cullMode = cullMode;
		effects[id] = e;
	}
	return id;
}

void ShaderMan::Destroy()
{
	for (auto it = nameToId.begin(); it != nameToId.end(); it++)
	{
		glDeleteProgram(it->second);
	}
	nameToId.clear();
}

void ShaderMan::Apply(SMID id)
{
	assert(id);
	auto it = effects.find(id);
	if (it != effects.end()) {
		afDepthStencilMode(it->second.depthStencilMode);
		afBlendMode(it->second.blendMode);
		afCullMode(it->second.cullMode);
		afHandleGLError(glUseProgram(id));
	}
}
