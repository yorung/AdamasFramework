local meshId = meshMan.Create("jiji.x")

local deg = 0
function Update()
	puzzle.Update();
--	MesBox("main.lua Update called")
--[[
	meshMan.Draw(meshId)

	matrixStack.Push()
	matrixStack.Translate(2.2, 0, 0)
	matrixStack.RotateY(deg)
	matrixStack.Scale(0.8, 0.8, 0.8)
	meshMan.Draw(meshId)
	matrixStack.Pop()

	matrixStack.Push()
	matrixStack.Translate(-2.2, 0, 0)
	matrixStack.RotateZ(deg)
	matrixStack.Scale(1.0 + math.sin(deg * math.pi / 180 * 2) * 0.5, 0.8, 1.0 + math.sin(deg * math.pi / 180 * 3) * 0.5)
	meshMan.Draw(meshId)
	matrixStack.Pop()

	deg = deg + 3
]]
end

puzzleEnabled = false

function DrawSprites()
	if puzzleEnabled then
		puzzle.Draw();
	end
end

--MesBox("main.lua loaded!")	-- toast

--[[ message box
if MessageBox("yes no box", "yesno") ==  "yes" then
	MessageBox("yes!", "ok")
else
	MessageBox("no!", "ok")
end
]]

AddMenu("Puzzle", "puzzleEnabled = true")
AddMenu("Msg", "MessageBox('message', 'ok')")
