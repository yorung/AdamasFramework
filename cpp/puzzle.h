class Puzzle
{
	int puzzle[16];
	SpriteCommands cmds;
	void TryMove(int x, int y);
public:
	Puzzle();
	void Update();
	void Draw();
};

extern Puzzle puzzle;
