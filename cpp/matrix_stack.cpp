#include "stdafx.h"

MatrixStack::MatrixStack()
{
	Reset();
}

void MatrixStack::Reset()
{
	std::stack<Mat> e;
	e.push(Mat());
	std::swap(stack, e);
}

void MatrixStack::Push()
{
	stack.push(stack.top());
}

void MatrixStack::Pop()
{
	assert(stack.size() > 1);
	if (stack.size() > 1) {
		stack.pop();
	}
}

void MatrixStack::Mul(const Mat& m)
{
	stack.top() = m * stack.top();
}
