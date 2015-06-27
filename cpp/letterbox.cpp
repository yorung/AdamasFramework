#include "stdafx.h"

LetterBox letterBox;

void LetterBox::LazyInit()
{
	if (shader) {
		return;
	}

	shader = shaderMan.Create("letterbox", stockObjects.GetFullScreenVertexAttributeLayout());
	assert(shader);
	afLayoutSamplerBindingManually(shader, "sampler", 0);

}

void LetterBox::Draw(AFRenderTarget& target, GLuint srcTex)
{
	LazyInit();

	target.BeginRenderToThis();
	afBindTextureToBindingPoint(srcTex, 0);
	afBindSamplerToBindingPoint(stockObjects.GetNoMipmapSampler(), 0);
	shaderMan.Apply(shader);
	stockObjects.ApplyFullScreenVAO();
	afDrawTriangleStrip(4);
}

void LetterBox::Destroy()
{
	shader = 0;
}
