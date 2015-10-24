function Update()
end

function Draw2D()
end

function Draw3D()
end

AddMenu("Puzzle", "dofile('lua/puzzle.lua')")
AddMenu("Msg", "MessageBox('message', 'ok')")
AddMenu("Jiji", "dofile('lua/jiji.lua')")
AddMenu("Instancing", "dofile('lua/instancing.lua')")

LoadSkyBox("hakodate.jpg", "photosphere")
