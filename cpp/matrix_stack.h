class MatrixStack
{
	std::stack<Mat> stack;
public:
	MatrixStack();
	void Reset();
	void Push();
	void Pop();
	void Mul(const Mat& m);
	const Mat& Get() { return stack.top(); }
};
