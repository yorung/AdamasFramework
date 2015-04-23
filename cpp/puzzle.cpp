#include "stdafx.h"

class Puzzle
{
	int panels[16];
	SpriteCommands cmds;
	void TryMove(int x, int y);
public:
	Puzzle();
	void Update();
	void Draw();
};

static Puzzle puzzle;

Puzzle::Puzzle()
{
	for (int i = 0; i < dimof(panels); i++) {
		panels[i] = i;
	}
	panels[0] = -1;
	srand((unsigned int)time(0));
	for (int i = 0; i < 10000; i++) {
		TryMove(rand() % 4, rand() % 4);
	}

	GetLuaBindFuncContainer().push_back([](lua_State* L) {
		static luaL_Reg inNamespaceFuncs[] = {
			{ "Update", [](lua_State* L) { puzzle.Update(); return 0; } },
			{ "Draw", [](lua_State* L) { puzzle.Draw(); return 0; } },
			{ nullptr, nullptr },
		};
		aflBindNamespace(L, "puzzle", inNamespaceFuncs);
	});
}

void Puzzle::TryMove(int x, int y)
{
	auto TryOne = [&](int xx, int yy) {
		if (xx < 0 || xx >= 4 || yy < 0 || yy >= 4) {
			return;
		}
		if (panels[xx + yy * 4] < 0) {
			std::swap(panels[x + y * 4], panels[xx + yy * 4]);
		}
	};
	TryOne(x, y - 1);
	TryOne(x, y + 1);
	TryOne(x - 1, y);
	TryOne(x + 1, y);
}

void Puzzle::Update()
{
	cmds.clear();
	SpriteCommand cmd;
	cmd.tex = texMan.Create("jiji.dds");

	const float pitch = 0.25f;
	const float sprW = 64;

	Vec2 mouse = systemMetrics.GetMousePos();
	auto isHit = [&](const Mat& m) {
		Vec3 v = transform(Vec3(mouse.x, mouse.y, 0), inv(m));
		return v.x >= 0 && v.y >= 0 && v.x < sprW && v.y < sprW;
	};
	Vec2 scrSize = systemMetrics.GetScreenSize();
	MatrixStack m;
	float mini = std::min(scrSize.x, scrSize.y);
	m.Mul(translate((scrSize.x - mini) / 2, (scrSize.y - mini) / 2, 0));
	m.Mul(scale(mini));
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			int spr = panels[x + y * 4];
			if (spr < 0) {
				continue;
			}
			int xx = spr % 4;
			int yy = spr / 4;
			cmd.quad = Vec4(xx * sprW, yy * sprW, xx * sprW + sprW, yy * sprW + sprW);
			m.Push();
			m.Mul(translate(pitch * x, pitch * y, 0));
			m.Mul(scale(0.25f / sprW));
			cmd.matW = m.Get();
			if (isHit(m.Get())) {
				cmd.color = 0xff0000ff;
				if (systemMetrics.mouseDown) {
					TryMove(x, y);
				}
			} else {
				cmd.color = 0xffffffff;
			}
			m.Pop();
			cmds.push_back(cmd);
		}
	}
}

void Puzzle::Draw()
{
	spriteRenderer.Draw(cmds);
}
