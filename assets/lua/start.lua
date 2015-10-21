function Update()
end

function DrawSprites()
end

AddMenu("Puzzle", "dofile('lua/puzzle.lua')")
AddMenu("Msg", "MessageBox('message', 'ok')")
AddMenu("Jiji", "dofile('lua/jiji.lua')")

LoadSkyBox("hakodate.jpg")
