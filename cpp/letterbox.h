#pragma once

class LetterBox
{
	AFRenderStates renderStates;
	void LazyInit();
public:
	void Draw(AFCommandList& cmd, AFRenderTarget& target, AFTexRef srcTex);
	void Destroy();
};

extern LetterBox letterBox;
