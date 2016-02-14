local matrixStack = MatrixStack()

local function ScaledMesh(fileName, scale)
	local mesh
	return
	{
		Draw = function(self, animId, time)
			if not mesh then
				mesh = Mesh(fileName)
			end
			matrixStack:Push()
				matrixStack:Scale(scale, scale, scale)
				mesh:Draw(matrixStack, animId, time)
			matrixStack:Pop()
		end
	}
end

local meshTbl =
{
	ScaledMesh("models/jiji.x", 0.60),
};

local meshFileNames =
{
	"models/jiji.x",
	"models/jiji.x",
	"models/nori.x",
	"models/jiji.x",
}

local blocks = {}
for k,v in pairs(meshFileNames) do
	local m = ScaledMesh(v, 0.4)
	blocks[ k ] = m
end

local m_frame = 0
local g_animId = 0

local function DrawOne( x, y, mesh )
	matrixStack:Push()
		matrixStack:Translate( x, 0.25, -y )
		local rot = m_frame
		matrixStack:RotateX( rot )
		matrixStack:RotateY( rot )
		matrixStack:RotateZ( rot )

		mesh:Draw(matrixStack, 0, m_frame * ( 1 / FPS ))
	matrixStack:Pop()
end


function Update()
	local eye = Vec3( 0, 7, -5 )
	local at = Vec3( 0, 3, 0 )
	LookAt( eye, at )

	if GetKeyCount(0x25) == 1 then	-- left
		g_animId = 0
		table.insert(meshTbl, 1, table.remove(meshTbl))
	end

	if GetKeyCount(0x27) == 1 then	-- right
		g_animId = 0
		table.insert(meshTbl, table.remove(meshTbl, 1))
	end

	if GetKeyCount(0x28) == 1 then	-- down
		g_animId = g_animId + 1
	end

	if GetKeyCount(0x26) == 1 then	-- up
		g_animId = g_animId - 1
	end

	m_frame = m_frame + 1
end

local function DrawBlocks()
	for x=-5, 5 do
		for y=-5, 5 do
			local i = x + y
			DrawOne( x, y, blocks[ i % #blocks  + 1 ]  )
		end
	end
end

function Draw3D()
	matrixStack:Push()
		matrixStack:Translate( 0, 3, 0 )
		matrixStack:RotateY( m_frame )
		meshTbl[1]:Draw(matrixStack, g_animId, m_frame / 60)
	matrixStack:Pop()

	DrawBlocks()
end

function Draw2D()
end
