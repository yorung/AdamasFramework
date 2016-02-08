local w = WaterSurface()
function Update()
	w:Update()
	LookAt(Vec3(10, 10, 10), Vec3(0, 0, 0))
end

function Draw2D()
	w:Draw()
end

function Draw3D()
end
