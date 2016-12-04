#include "stdafx.h"

static void SetSamplerLayoutByName(GLuint program)
{
	for(int i = 0; i <= 9; i++)
	{
		char name[] = {'s', (char)('0' + i), '\0'};
		afLayoutSamplerBindingManually(program, name, i);
	}
}

static GLuint CompileShader(int type, const char *fileName)
{
	GLuint shader = glCreateShader(type);

	void* img = LoadFile(fileName);
	if (!img)
	{
		aflog("shader file %s not found", fileName);
		return 0;
	}
	glShaderSource(shader, 1, (const char**)&img, NULL);
	glCompileShader(shader);
	free(img);

	int result = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
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
	else
	{
		aflog("CompileShader(%s) succeess. id=%d\n", fileName, shader);
	}
	return shader;
}

static GLuint afCompileGLSL(const char* name, const InputElement elements[], int numElements)
{
	char buf[256];
	snprintf(buf, dimof(buf), "glsl/%s.vert", name);
	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, buf);
	if (!vertexShader)
	{
		return 0;
	}
	snprintf(buf, dimof(buf), "glsl/%s.frag", name);
	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, buf);
	if (!fragmentShader)
	{
		return 0;
	}
	GLuint program = glCreateProgram();
	for (int i = 0; i < numElements; i++)
	{
		glBindAttribLocation(program, i, elements[i].name);
	}

	glAttachShader(program, vertexShader);
	glDeleteShader(vertexShader);
	glAttachShader(program, fragmentShader);
	glDeleteShader(fragmentShader);
	glLinkProgram(program);

	GLint status = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
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
	else
	{
		aflog("CreateProgram(%s) succeess. id=%d\n", name, program);
	}
	SetSamplerLayoutByName(program);
	return program;
}

IBOID afCreateIndexBuffer(int numIndi, const AFIndex* indi)
{
	IBOID ibo;
	afHandleGLError(glGenBuffers(1, &ibo.x));
	afHandleGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	afHandleGLError(glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndi * sizeof(AFIndex), &indi[0], GL_STATIC_DRAW));
	afHandleGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	return ibo;
}

VBOID afCreateVertexBuffer(int size, const void* buf)
{
	VBOID vbo;
	afHandleGLError(glGenBuffers(1, &vbo.x));
	afHandleGLError(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	afHandleGLError(glBufferData(GL_ARRAY_BUFFER, size, buf, GL_STATIC_DRAW));
	afHandleGLError(glBindBuffer(GL_ARRAY_BUFFER, 0));
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

#ifdef AF_GLES31
SSBOID afCreateSSBO(int size)
{
	SSBOID name;
	glGenBuffers(1, &name.x);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, name);
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	return name;
}

UBOID afCreateUBO(int size, const void* buf)
{
	UBOID name;
	glGenBuffers(1, &name.x);
	glBindBuffer(GL_UNIFORM_BUFFER, name);
	glBufferData(GL_UNIFORM_BUFFER, size, buf, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return name;
}
#endif

// without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding)
{
	afHandleGLError(glUseProgram(program));
	GLint location = glGetUniformLocation(program, name);
	if (location >= 0)
	{
		afHandleGLError(glUniform1i(location, samplerBinding));
	}
}
#if 0
void afLayoutSSBOBindingManually(GLuint program, const GLchar* name, GLuint storageBlockBinding)
{
	glShaderStorageBlockBinding(program, glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name), storageBlockBinding);
}

void afLayoutUBOBindingManually(GLuint program, const GLchar* name, GLuint uniformBlockBinding)
{
	glUniformBlockBinding(program, glGetUniformBlockIndex(program, name), uniformBlockBinding);
}
#endif

#ifdef AF_GLES31
void afBindBuffer(SSBOID ssbo, GLuint storageBlockBinding)
{
	GLint prev;
	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_BINDING, &prev);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, storageBlockBinding, ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, prev);
}

void afBindBuffer(UBOID ubo, GLuint uniformBlockBinding)
{
	GLint prev;
	glGetIntegerv(GL_UNIFORM_BUFFER_BINDING, &prev);
	glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlockBinding, ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, prev);
}
#endif

void afUpdateUniformVariable(GLuint program, int size, const void* buffer, const char* name)
{
	assert(size % 16 == 0);
	GLint location = glGetUniformLocation(program, name);
	if (location >= 0)
	{
		afHandleGLError(glUniform4fv(location, size / 16, (GLfloat*)buffer));
	}
}

void afBindTexture(GLuint tex, GLuint textureBindingPoint)
{
	afHandleGLError(glActiveTexture(GL_TEXTURE0 + textureBindingPoint));
	afHandleGLError(glBindTexture(GL_TEXTURE_2D, tex));
}

void afBindCubeMap(GLuint tex, GLuint textureBindingPoint)
{
	afHandleGLError(glActiveTexture(GL_TEXTURE0 + textureBindingPoint));
	afHandleGLError(glBindTexture(GL_TEXTURE_CUBE_MAP, tex));
}

void afWriteTexture(SRVID srv, const TexDesc& desc, const void* buf)
{
	glBindTexture(GL_TEXTURE_2D, srv);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, desc.size.x, desc.size.y, GL_RGBA, GL_UNSIGNED_BYTE, buf);
	glBindTexture(GL_TEXTURE_2D, 0);
}

static void CreateTextureInternal(AFFormat format, const IVec2& size, void* img)
{
	auto gen = [=](GLint internalFormat, GLint format, GLenum type) {
		afHandleGLError(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, format, type, img));
	};
	switch (format) {
	case AFF_R8G8B8A8_UNORM:
		gen(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		break;
	case AFF_R8G8B8A8_UNORM_SRGB:
#ifdef AF_GLES31
		gen(GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE);
#endif
		break;
	case AFF_R5G6B5_UINT:
		gen(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
		break;

	case AFF_R16G16B16A16_FLOAT:
#ifdef AF_GLES31
		gen(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
#else
		gen(GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES);
#endif
		break;

	case AFF_R32G32B32A32_FLOAT:
#ifdef AF_GLES31
		gen(GL_RGBA32F, GL_RGBA, GL_FLOAT);
#else
		gen(GL_RGBA, GL_RGBA, GL_FLOAT);
#endif
		break;

#ifdef AF_GLES31
	case AFF_DEPTH:
		gen(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
		break;
	case AFF_DEPTH_STENCIL:
		gen(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
		break;
#endif
	default:
		assert(0);
		break;
	}
}

SRVID afCreateDynamicTexture(AFFormat format, const IVec2& size)
{
	SRVID texture;
	glGenTextures(1, &texture.x);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	CreateTextureInternal(format, size, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

SRVID afCreateTexture2D(AFFormat format, const TexDesc& desc, int mipCount, const AFTexSubresourceData datas[])
{
	GLenum target = desc.isCubeMap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
	GLenum targetFace = desc.isCubeMap ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : GL_TEXTURE_2D;

	SRVID texture;
	glGenTextures(1, &texture.x);
	glBindTexture(target, texture);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int idx = 0;
	for (int a = 0; a < desc.arraySize; a++) {
		for (int m = 0; m < mipCount; m++) {
			int w = std::max(1, desc.size.x >> m);
			int h = std::max(1, desc.size.y >> m);
			if (format == AFF_R8G8B8A8_UNORM) {
				afHandleGLError(glTexImage2D(targetFace + a, m, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, datas[idx].ptr));
			} else if (format == AFF_B8G8R8A8_UNORM) {
#ifdef _MSC_VER	// GL_EXT_texture_format_BGRA8888 is an extension for OpenGL ES, so should use this only on Windows.
				afHandleGLError(glTexImage2D(targetFace + a, m, GL_BGRA_EXT, w, h, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, datas[idx].ptr));
#endif
			} else {
				afHandleGLError(glCompressedTexImage2D(targetFace + a, m, format, w, h, 0, datas[idx].pitchSlice, datas[idx].ptr));
			}
			idx++;
		}
	}
	if (mipCount == 1) {
		if (format == AFF_R8G8B8A8_UNORM || format == AFF_B8G8R8A8_UNORM) {
			afHandleGLError(glGenerateMipmap(target));
		} else {
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	// temporary disable mipmap
		}
	}

	glBindTexture(target, 0);
	return texture;
}

void afDrawIndexed(PrimitiveTopology primitiveTopology, int numIndices, int start, int instanceCount)
{
#ifdef AF_GLES31
	afHandleGLError(glDrawElementsInstanced/*BaseVertex*/(primitiveTopology, numIndices, AFIndexTypeToDevice, (void*)(start * sizeof(AFIndex)), instanceCount/*, cmd.baseVertex*/));
#else
	assert(instanceCount == 1);
	afHandleGLError(glDrawElements(primitiveTopology, numIndices, AFIndexTypeToDevice, (void*)(start * sizeof(AFIndex))));
#endif
}

void afDraw(PrimitiveTopology primitiveTopology, int numVertices, int start, int instanceCount)
{
#ifdef AF_GLES31
	afHandleGLError(glDrawArraysInstanced(primitiveTopology, start * sizeof(AFIndex), numVertices, instanceCount));
#else
	assert(instanceCount == 1);
	afHandleGLError(glDrawArrays(primitiveTopology, start * sizeof(AFIndex), numVertices));
#endif
}

void afSetVertexAttributes(const InputElement elements[], int numElements, int numBuffers, VBOID const vertexBufferIds[], const int strides[])
{
	for (int i = 0; i < numElements; i++) {
		const InputElement& d = elements[i];
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIds[d.inputSlot]);
		GLenum r = glGetError();
		if (r != GL_NO_ERROR) {
			aflog("glBindBuffer error! i=%d inputSlot=%d vbo=%d\n", i, d.inputSlot, vertexBufferIds[d.inputSlot].x);
		}
		afHandleGLError(glEnableVertexAttribArray(i));
		switch (d.format) {
		case AFF_R32_FLOAT:
		case AFF_R32G32_FLOAT:
		case AFF_R32G32B32_FLOAT:
		case AFF_R32G32B32A32_FLOAT:
			afHandleGLError(glVertexAttribPointer(i, d.format - AFF_R32_FLOAT + 1, GL_FLOAT, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case AFF_R8_UNORM:
		case AFF_R8G8_UNORM:
		case AFF_R8G8B8_UNORM:
		case AFF_R8G8B8A8_UNORM:
			afHandleGLError(glVertexAttribPointer(i, d.format - AFF_R8_UNORM + 1, GL_UNSIGNED_BYTE, GL_TRUE, strides[d.inputSlot], (void*)d.offset));
			break;
		case AFF_R8_UINT:
		case AFF_R8G8_UINT:
		case AFF_R8G8B8_UINT:
		case AFF_R8G8B8A8_UINT:
			afHandleGLError(glVertexAttribPointer(i, d.format - AFF_R8_UINT + 1, GL_UNSIGNED_BYTE, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case AFF_R16_UINT:
		case AFF_R16G16_UINT:
		case AFF_R16G16B16_UINT:
		case AFF_R16G16B16A16_UINT:
			afHandleGLError(glVertexAttribPointer(i, d.format - AFF_R16_UINT + 1, GL_UNSIGNED_SHORT, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case AFF_R32_UINT:
		case AFF_R32G32_UINT:
		case AFF_R32G32B32_UINT:
		case AFF_R32G32B32A32_UINT:
			afHandleGLError(glVertexAttribPointer(i, d.format - AFF_R32_UINT + 1, GL_UNSIGNED_INT, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		default:
			assert(0);
			break;
		}
#ifdef AF_GLES31
		if (d.perInstance) {
			afHandleGLError(glVertexAttribDivisor(i, 1));
		}
#endif
	}
	afHandleGLError(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void afSetIndexBuffer(IBOID indexBuffer)
{
	afHandleGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer));
}

#ifdef AF_GLES31
VAOID afCreateVAO(const InputElement elements[], int numElements, int numBuffers, VBOID const vertexBufferIds[], const int strides[], IBOID ibo)
{
	VAOID vao;
	afHandleGLError(glGenVertexArrays(1, &vao.x));
	afHandleGLError(glBindVertexArray(vao));
	afSetVertexAttributes(elements, numElements, numBuffers, vertexBufferIds, strides);
	afHandleGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	afHandleGLError(glBindVertexArray(0));
	return vao;
}
#endif

void afBlendMode(uint32_t flags)
{
	if (flags & AFRS_ALPHA_BLEND)
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
	else
	{
		glDisable(GL_BLEND);
	}
}

void afDepthStencilMode(uint32_t flags)
{
	if (flags & AFRS_DEPTH_ENABLE)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_GREATER);
	}
	else if (flags & AFRS_DEPTH_CLOSEREQUAL_READONLY)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_GEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_ALWAYS);
	}
}

void afCullMode(uint32_t flags)
{
	if (flags & AFRS_CULL_CCW)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);
	}
	else if (flags & AFRS_CULL_CW)
	{
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

#ifdef AF_GLES31
SAMPLERID afCreateSampler(SamplerType type)
{
	GLint wrap = (type & 0x01) ? GL_CLAMP_TO_EDGE : GL_REPEAT;
	int filter = type >> 1;
	SAMPLERID id;
	glGenSamplers(1, &id.x);
	glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrap);
	switch(filter) {
	case 2:	// mipmap
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		break;
	case 1:	// linear
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case 0:	// point
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	}
	return id;
}
#endif

GLenum _afHandleGLError(const char* file, const char* func, int line, const char* command)
{
	GLenum r = glGetError();
	if (r != GL_NO_ERROR) {
		const char *err = nullptr;
		switch (r) {
#define E(er) case er: err = #er; break
		E(GL_INVALID_ENUM);
		E(GL_INVALID_VALUE);
		E(GL_INVALID_OPERATION);
		E(GL_INVALID_FRAMEBUFFER_OPERATION);
#undef E
		default:
			aflog("%s %s(%d): err=%d %s\n", file, func, line, r, command);
			return r;
		}
		aflog("%s %s(%d): %s %s\n", file, func, line, err, command);
	}
	return r;
}

void afDumpCaps()
{
	aflog("GL_VERSION = %s\n", (char*)glGetString(GL_VERSION));
	aflog("GL_RENDERER = %s\n", (char*)glGetString(GL_RENDERER));
	aflog("GL_VENDOR = %s\n", (char*)glGetString(GL_VENDOR));
	aflog("GL_SHADING_LANGUAGE_VERSION = %s\n", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
#ifdef AF_GLES31
#define _(x) do { GLint i; glGetIntegerv(x, &i); aflog(#x " = %d\n", i); } while(0)
	aflog("------ Uniform Buffers\n");
	_(GL_MAX_UNIFORM_BUFFER_BINDINGS);
	_(GL_MAX_VERTEX_UNIFORM_BLOCKS);
	_(GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
	_(GL_MAX_COMPUTE_UNIFORM_BLOCKS);
	//	_(GL_MAX_GEOMETRY_UNIFORM_BLOCKS);
	_(GL_MAX_COMBINED_UNIFORM_BLOCKS);
	_(GL_MAX_UNIFORM_BLOCK_SIZE);
	//	_(GL_MAX_COMBINED_GEOMETRY_UNIFORM_COMPONENTS);
	_(GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS);
	_(GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS);
	aflog("------ Uniform Variables\n");
	_(GL_MAX_VERTEX_UNIFORM_COMPONENTS);
	_(GL_MAX_VERTEX_UNIFORM_VECTORS);
	_(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS);
	_(GL_MAX_FRAGMENT_UNIFORM_VECTORS);
	aflog("------ SSBO\n");
	_(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
	_(GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
	_(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
	_(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS);
//	_(GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS);
#undef _
	aflog("------ GL_EXTENSIONS\n");

	GLint num;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	for (int i = 0; i < num; i++)
	{
		const GLubyte* ext = glGetStringi(GL_EXTENSIONS, i);
		aflog("%s\n", ext);
	}
#endif
}

void afDumpIsEnabled()
{
#define _(x) do { printf(#x " = %s\n", (glIsEnabled(x) ? "true" : "false")); } while(0)
	_(GL_CULL_FACE);
	_(GL_DEPTH_TEST);
	_(GL_STENCIL_TEST);
#undef _
}

void AFRenderTarget::Destroy()
{
	afSafeDeleteTexture(texColor);
	afSafeDeleteTexture(texDepth);
	if (framebufferObject) {
		glDeleteFramebuffers(1, &framebufferObject);
		framebufferObject = 0;
	}
	if (renderbufferObject) {
		glDeleteRenderbuffers(1, &renderbufferObject);
		renderbufferObject = 0;
	}
}

void AFRenderTarget::InitForDefaultRenderTarget()
{
	Destroy();
	texSize = systemMisc.GetScreenSize();
}

void AFRenderTarget::Init(IVec2 size, AFFormat colorFormat, AFFormat depthStencilFormat)
{
	Destroy();
	texSize = size;
	texColor = afCreateDynamicTexture(colorFormat, size);
	if (texColor == 0) {
		aflog("AFRenderTarget::Init: cannot create dynamic texture. format=%d", colorFormat);
	}
	if (depthStencilFormat != AFF_INVALID) {
		texDepth = afCreateDynamicTexture(depthStencilFormat, size);
	}

	afHandleGLError(glGenFramebuffers(1, &framebufferObject));
	afHandleGLError(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
	afHandleGLError(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColor, 0));
	if (texDepth) {
#ifdef AF_GLES31
		afHandleGLError(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, texDepth, 0));
#endif
	}
	afHandleGLError(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void AFRenderTarget::BeginRenderToThis()
{
	afHandleGLError(glViewport(0, 0, texSize.x, texSize.y));
	afHandleGLError(glBindFramebuffer(GL_FRAMEBUFFER, framebufferObject));
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	assert(status == GL_FRAMEBUFFER_COMPLETE);
	afHandleGLError(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

#ifdef AF_GLES31
IVec2 afGetTextureSize(SRVID tex)
{
	GLint w, h;
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);
	return IVec2(w, h);
}

IVec2 afGetRenderbufferSize(GLuint renderbuffer)
{
	GLint w, h;
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
	return IVec2(w, h);
}

#endif

void afClear()
{
	afHandleGLError(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}

void AFRenderStates::Create(const char* shaderName, int numInputElements, const InputElement* inputElements, uint32_t flags_, int numSamplerTypes_, const SamplerType samplerTypes_[])
{
	shaderId = afCompileGLSL(shaderName, inputElements, numInputElements);
	elements = inputElements;
	numElements = numInputElements;
	flags = flags_;
	numSamplerTypes = numSamplerTypes_;
	samplerTypes = samplerTypes_;
}

static PrimitiveTopology RenderFlagsToPrimitiveTopology(uint32_t flags)
{
	if (flags & AFRS_PRIMITIVE_TRIANGLELIST)
	{
		return PT_TRIANGLELIST;
	}
	else if (flags & AFRS_PRIMITIVE_LINELIST)
	{
		return PT_LINELIST;
	}
	return PT_TRIANGLESTRIP;
}

PrimitiveTopology AFRenderStates::GetPrimitiveTopology() const
{
	return RenderFlagsToPrimitiveTopology(flags);
}

void AFRenderStates::Apply() const
{
	afHandleGLError(glUseProgram(shaderId));
	afBlendMode(flags);
	afDepthStencilMode(flags);
	afCullMode(flags);
#ifdef AF_GLES31
	for (int i = 0; i < numSamplerTypes; i++)
	{
		afSetSampler(samplerTypes[i], i);
	}
#endif
}

void AFRenderStates::Destroy()
{
	if (shaderId != 0)
	{
		glDeleteProgram(shaderId);
		shaderId = 0;
	}
}

#ifdef AF_GLES31
void afSetSampler(SamplerType type, int slot)
{
	afBindSamplerToBindingPoint(stockObjects.GetBuiltInSampler(type), slot);
}
#endif
