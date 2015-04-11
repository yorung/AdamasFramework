#include "stdafx.h"

MatrixStack matrixStack;

MatrixStack::MatrixStack()
{
	Reset();
}

void MatrixStack::Reset()
{
	std::stack<Mat> e;
	std::swap(stack, e);
	matrixMan.Set(MatrixMan::WORLD, Mat());
}

void MatrixStack::Push()
{
	Mat m;
	matrixMan.Get(MatrixMan::WORLD, m);
	stack.push(m);
}

void MatrixStack::Pop()
{
	assert(!stack.empty());
	if (stack.empty()) {
		return;
	}
	matrixMan.Set(MatrixMan::WORLD, stack.top());
	stack.pop();
}

void MatrixStack::Mul(const Mat& m)
{
	Mat w;
	matrixMan.Get(MatrixMan::WORLD, w);
	matrixMan.Set(MatrixMan::WORLD, m * w);
}
