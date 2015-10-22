--local meshId = meshMan.Create("jiji.x")
local mesh = Mesh("jiji.x")
local deg = 0
local elapsed = 0;

function Update()
	mesh:Draw(0, elapsed)

	matrixStack.Push()
	matrixStack.Translate(2.2, 0, 0)
	matrixStack.RotateY(deg)
	matrixStack.Scale(0.8, 0.8, 0.8)
	mesh:Draw(0, elapsed)
	matrixStack.Pop()

	matrixStack.Push()
	matrixStack.Translate(-2.2, 0, 0)
	matrixStack.RotateZ(deg)
	matrixStack.Scale(1.0 + math.sin(deg * math.pi / 180 * 2) * 0.5, 0.8, 1.0 + math.sin(deg * math.pi / 180 * 3) * 0.5)
	mesh:Draw(0, elapsed)
	matrixStack.Pop()

	deg = deg + 3
	elapsed = elapsed + 0.1666666

	if GetKeyCount(32) >= 0 then
		LookAt(Vec3(10, 10, 10), Vec3(0, 0, 0))
	end
end

function DrawSprites()
end
