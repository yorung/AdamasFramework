class MatrixStack
{
	std::stack<Mat> stack;
	void Apply();
public:
	MatrixStack();
	void Reset();
	void Push();
	void Pop();
	void Mul(const Mat& m);
};

extern MatrixStack matrixStack;
