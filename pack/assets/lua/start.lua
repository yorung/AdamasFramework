function Update()
end

function Draw2D()
end

function Draw3D()
end

function Destroy()
end

function RunModule(moduleFileName)
	Destroy()
	collectgarbage("collect")
	dofile(moduleFileName)
end

AddMenu("Ocean", "RunModule('lua/ocean.lua')")
AddMenu("Picking", "RunModule('lua/picking.lua')")
AddMenu("Hasami Shogi", "RunModule('lua/hasami_shogi/main.lua')")
AddMenu("Puzzle", "RunModule('lua/puzzle.lua')")
AddMenu("Water(ES2)", "RunModule('lua/water_surface_es2.lua')")
AddMenu("Water(ES3)", "RunModule('lua/water_surface_es3.lua')")
AddMenu("Msg", "MessageBox('message', 'ok')")
AddMenu("Jiji", "RunModule('lua/jiji.lua')")
AddMenu("Instancing", "RunModule('lua/instancing.lua')")
AddMenu("LittlePlanet", "LoadSkyBox('hakodate.jpg', 'projection_equirectangular_to_stereographic')")

LoadSkyBox("hakodate.jpg", "sky_photosphere")

dofile('lua/hasami_shogi/main.lua')
--dofile('lua/picking.lua')
