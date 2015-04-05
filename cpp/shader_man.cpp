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
	snprintf(buf, dimof(buf), "shaders/%s.vert", name);
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, buf);
	if (!vertexShader) {
		return 0;
	}
	snprintf(buf, dimof(buf), "shaders/%s.frag", name);
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

ShaderMan::SMID ShaderMan::Create(const char *name)
{
	NameToId::iterator it = nameToId.find(name);
	if (it != nameToId.end())
	{
		return it->second;
	}
	return nameToId[name] = CreateProgram(name);
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
	glUseProgram(id);
}
