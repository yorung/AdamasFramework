class Puzzle
{
	int puzzle[16];
	SpriteCommands cmds;
public:
	Puzzle();
	void Update();
	void Draw();
	void TryMove(int i);
};

extern Puzzle puzzle;
