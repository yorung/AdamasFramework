#include "stdafx.h"

MatrixMan matrixMan;

const Mat& MatrixMan::Get(Type type) const
{
	return matrices[type];
}

void MatrixMan::Set(Type type, const Mat& m)
{
	matrices[type] = m;
}
