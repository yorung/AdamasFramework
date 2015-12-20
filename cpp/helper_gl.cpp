#include "stdafx.h"

IBOID afCreateIndexBuffer(const AFIndex* indi, int numIndi)
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

UBOID afCreateUBO(int size)
{
	UBOID name;
	glGenBuffers(1, &name.x);
	glBindBuffer(GL_UNIFORM_BUFFER, name);
	glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	return name;
}
#endif

// without "binding" Layout Qualifier
void afLayoutSamplerBindingManually(GLuint program, const GLchar* name, GLuint samplerBinding)
{
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, name), samplerBinding);
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
#endif

void afBindTextureToBindingPoint(GLuint tex, GLuint textureBindingPoint)
{
	afHandleGLError(glActiveTexture(GL_TEXTURE0 + textureBindingPoint));
	afHandleGLError(glBindTexture(GL_TEXTURE_2D, tex));
}

void afBindCubeMapToBindingPoint(GLuint tex, GLuint textureBindingPoint)
{
	afHandleGLError(glActiveTexture(GL_TEXTURE0 + textureBindingPoint));
	afHandleGLError(glBindTexture(GL_TEXTURE_CUBE_MAP, tex));
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
	auto gen = [=](GLint internalFormat, GLint format, GLenum type) {
		afHandleGLError(glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, type, nullptr));
	};
	switch (format) {
	case AFDT_R8G8B8A8_UINT:
		gen(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		break;
	case AFDT_R5G6B5_UINT:
		gen(GL_RGB, GL_RGB, GL_UNSIGNED_SHORT_5_6_5);
		break;

	case AFDT_R16G16B16A16_FLOAT:
#ifdef AF_GLES31
		gen(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT);
#else
		gen(GL_RGBA, GL_RGBA, GL_HALF_FLOAT_OES);
#endif
		break;

	case AFDT_R32G32B32A32_FLOAT:
#ifdef AF_GLES31
		gen(GL_RGBA32F, GL_RGBA, GL_FLOAT);
#else
		gen(GL_RGBA, GL_RGBA, GL_FLOAT);
#endif
		break;

#ifdef AF_GLES31
	case AFDT_DEPTH:
		gen(GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
		break;
	case AFDT_DEPTH_STENCIL:
		gen(GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8);
		break;
#endif
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

GLuint afCreateWhiteTexture()
{
	uint32_t col = 0xffffffff;
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &col);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texture;
}

void afDrawIndexedTriangleList(int numIndices, int start)
{
	glDrawElements(GL_TRIANGLES, numIndices, AFIndexTypeToDevice, (void*)(start));
}

void afDrawIndexedTriangleStrip(int numIndices, int start)
{
	glDrawElements(GL_TRIANGLE_STRIP, numIndices, AFIndexTypeToDevice, (void*)(start));
}

void afDrawTriangleStrip(int numVertices, int start)
{
	glDrawArrays(GL_TRIANGLE_STRIP, start, numVertices);
}

void afSetVertexAttributes(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides)
{
	for (int i = 0; i < numElements; i++) {
		const InputElement& d = elements[i];
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferIds[d.inputSlot]);
		GLenum r = glGetError();
		if (r != GL_NO_ERROR) {
			aflog("glBindBuffer error! i=%d inputSlot=%d vbo=%d\n", i, d.inputSlot, vertexBufferIds[d.inputSlot].x);
		}
		GLint h = d.name ? glGetAttribLocation(program, d.name) : d.attributeIndex;
		if (h == -1) {
			continue;
		}
		afHandleGLError(glEnableVertexAttribArray(h));
		switch (d.format) {
		case SF_R32_FLOAT:
		case SF_R32G32_FLOAT:
		case SF_R32G32B32_FLOAT:
		case SF_R32G32B32A32_FLOAT:
			afHandleGLError(glVertexAttribPointer(h, d.format - SF_R32_FLOAT + 1, GL_FLOAT, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case SF_R8_UNORM:
		case SF_R8G8_UNORM:
		case SF_R8G8B8_UNORM:
		case SF_R8G8B8A8_UNORM:
			afHandleGLError(glVertexAttribPointer(h, d.format - SF_R8_UNORM + 1, GL_UNSIGNED_BYTE, GL_TRUE, strides[d.inputSlot], (void*)d.offset));
			break;
		case SF_R8_UINT_TO_FLOAT:
		case SF_R8G8_UINT_TO_FLOAT:
		case SF_R8G8B8_UINT_TO_FLOAT:
		case SF_R8G8B8A8_UINT_TO_FLOAT:
			afHandleGLError(glVertexAttribPointer(h, d.format - SF_R8_UINT_TO_FLOAT + 1, GL_UNSIGNED_BYTE, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case SF_R8_UINT:
		case SF_R8G8_UINT:
		case SF_R8G8B8_UINT:
		case SF_R8G8B8A8_UINT:
#ifdef AF_GLES31
			afHandleGLError(glVertexAttribIPointer(h, d.format - SF_R8_UINT + 1, GL_UNSIGNED_BYTE, strides[d.inputSlot], (void*)d.offset));
#endif
			break;
		case SF_R16_UINT_TO_FLOAT:
		case SF_R16G16_UINT_TO_FLOAT:
		case SF_R16G16B16_UINT_TO_FLOAT:
		case SF_R16G16B16A16_UINT_TO_FLOAT:
			afHandleGLError(glVertexAttribPointer(h, d.format - SF_R16_UINT_TO_FLOAT + 1, GL_UNSIGNED_SHORT, GL_FALSE, strides[d.inputSlot], (void*)d.offset));
			break;
		case SF_R16_UINT:
		case SF_R16G16_UINT:
		case SF_R16G16B16_UINT:
		case SF_R16G16B16A16_UINT:
#ifdef AF_GLES31
			afHandleGLError(glVertexAttribIPointer(h, d.format - SF_R16_UINT + 1, GL_UNSIGNED_SHORT, strides[d.inputSlot], (void*)d.offset));
#endif
			break;
		case SF_R32_UINT:
		case SF_R32G32_UINT:
		case SF_R32G32B32_UINT:
		case SF_R32G32B32A32_UINT:
#ifdef AF_GLES31
			afHandleGLError(glVertexAttribIPointer(h, d.format - SF_R32_UINT + 1, GL_UNSIGNED_INT, strides[d.inputSlot], (void*)d.offset));
#endif
			break;
		}
#ifdef AF_GLES31
		if (d.perInstance) {
			afHandleGLError(glVertexAttribDivisor(h, 1));
		}
#endif
	}
	afHandleGLError(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

#ifdef AF_GLES31
GLuint afCreateVAO(GLuint program, const InputElement elements[], int numElements, int numBuffers, VBOID const *vertexBufferIds, const GLsizei* strides, IBOID ibo)
{
	GLuint vao;
	afHandleGLError(glGenVertexArrays(1, &vao));
	afHandleGLError(glBindVertexArray(vao));
	afSetVertexAttributes(program, elements, numElements, numBuffers, vertexBufferIds, strides);
	afHandleGLError(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
	afHandleGLError(glBindVertexArray(0));
	return vao;
}
#endif

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

void afBlendMode(BlendMode mode)
{
	switch(mode) {
	case BM_ALPHA:
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case BM_NONE:
		glDisable(GL_BLEND);
		break;
	}
}

void afDepthStencilMode(bool depth)
{
	if (depth) {
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_GEQUAL);
		glClearDepthf(0);
	} else {
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glDepthFunc(GL_ALWAYS);
	}
}

void afEnableBackFaceCulling(bool cullBack)
{
	if (cullBack) {
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
//		glCullFace(GL_FRONT);
//		glFrontFace(GL_CCW);
	} else {
		glDisable(GL_CULL_FACE);
	}
}

SAMPLERID afCreateSampler(SamplerFilter filter, SamplerWrap wrap)
{
	SAMPLERID id = 0;
	glGenSamplers(1, &id);
	glSamplerParameteri(id, GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(id, GL_TEXTURE_WRAP_T, wrap);
	switch(filter) {
	case SF_MIPMAP:
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	case SF_LINEAR:
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		break;
	case SF_POINT:
		glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		break;
	}
	return id;
}

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
	aflog("------ GL_EXTENSIONS\n");

	GLint num;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	for (int i = 0; i < num; i++) {
		const GLubyte* ext = glGetStringi(GL_EXTENSIONS, i);
		aflog("%s\n", ext);
	}

	aflog("------ glGet\n");
#define _(x) do { GLint i; glGetIntegerv(x, &i); aflog(#x " = %d\n", i); } while(0)
	_(GL_MAX_UNIFORM_BUFFER_BINDINGS);
	_(GL_MAX_UNIFORM_BLOCK_SIZE);
	_(GL_MAX_VERTEX_UNIFORM_BLOCKS);
	_(GL_MAX_FRAGMENT_UNIFORM_BLOCKS);
//	_(GL_MAX_GEOMETRY_UNIFORM_BLOCKS);

	_(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
	_(GL_MAX_SHADER_STORAGE_BLOCK_SIZE);
	_(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS);
#undef _
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

void AFRenderTarget::Init(ivec2 size, AFDTFormat colorFormat, AFDTFormat depthStencilFormat = AFDT_DEPTH_STENCIL)
{
	Destroy();
	texSize = size;
	texColor = afCreateDynamicTexture(size.x, size.y, colorFormat);
	if (texColor == 0) {
		aflog("AFRenderTarget::Init: cannot create dynamic texture. format=%d", colorFormat);
	}
	if (depthStencilFormat != AFDT_INVALID) {
		texDepth = afCreateDynamicTexture(size.x, size.y, depthStencilFormat);
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
ivec2 afGetTextureSize(GLuint tex)
{
	GLint w, h;
	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, 0);
	return ivec2(w, h);
}

ivec2 afGetRenderbufferSize(GLuint renderbuffer)
{
	GLint w, h;
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &w);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &h);
	return ivec2(w, h);
}

#endif
