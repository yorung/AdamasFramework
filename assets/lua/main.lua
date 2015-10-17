function Update()
end

function DrawSprites()
end

--MesBox("main.lua loaded!")	-- toast

--[[ message box
if MessageBox("yes no box", "yesno") ==  "yes" then
	MessageBox("yes!", "ok")
else
	MessageBox("no!", "ok")
end
]]

AddMenu("Puzzle", "dofile('lua/puzzle.lua')")
AddMenu("Msg", "MessageBox('message', 'ok')")
AddMenu("Jiji", "dofile('lua/jiji.lua')")
