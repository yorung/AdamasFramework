class MatrixMan
{
public:
	enum Type
	{
		VIEW,
		PROJ,

		MAX
	};
	Mat matrices[MAX];
public:
	const Mat& Get(Type type) const;
	void Set(Type type, const Mat& m);
};

extern MatrixMan matrixMan;
