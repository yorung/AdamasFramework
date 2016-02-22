function Update()
end

function Draw2D()
end

function Draw3D()
end

AddMenu("Picking", "dofile('lua/picking.lua')")
AddMenu("Hasami Shogi", "dofile('lua/hasami_shogi/main.lua')")
AddMenu("Puzzle", "dofile('lua/puzzle.lua')")
AddMenu("WaterSurfaceClassic", "dofile('lua/water_surface_classic.lua')")
AddMenu("WaterSurface", "dofile('lua/water_surface.lua')")
AddMenu("Msg", "MessageBox('message', 'ok')")
AddMenu("Jiji", "dofile('lua/jiji.lua')")
AddMenu("Instancing", "dofile('lua/instancing.lua')")
AddMenu("LittlePlanet", "LoadSkyBox('hakodate.jpg', 'projection_equirectangular_to_stereographic')")

LoadSkyBox("hakodate.jpg", "sky_photosphere")

dofile('lua/hasami_shogi/main.lua')
--dofile('lua/picking.lua')
