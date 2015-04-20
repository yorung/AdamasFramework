#include "stdafx.h"

Puzzle puzzle;

Puzzle::Puzzle()
{
	for (int i = 0; i < dimof(puzzle); i++) {
		puzzle[i] = i;
	}
	puzzle[0] = -1;
	std::random_shuffle(puzzle, puzzle + 16);
}

void Puzzle::TryMove(int i)
{
	auto TryOne = [&](int idx) {
		if (idx < 0 || idx >= 16) {
			return;
		}
		if (puzzle[idx] >= 0) {
			return;
		}
		std::swap(puzzle[idx], puzzle[i]);
	};
	TryOne(i - 4);
	TryOne(i + 4);
	TryOne(i - 1);
	TryOne(i + 1);
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
			int spr = puzzle[x + y * 4];
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
					TryMove(x + y * 4);
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
