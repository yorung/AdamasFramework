local gridTools = require("lua/hasami_shogi/grid_tools")

local gridRenderer = GridRenderer(9, 1)

LoadSkyBox("hakodate.jpg", "sky_photosphere")

local matrixStack = MatrixStack()

local jiji = Mesh("models/jiji.x")
local nori = Mesh("models/nori.x")
local pointer = Mesh("models/enemy.x")

local function WrapDrawer(drawer)
	return function(x, y, z)
		x = x or 0
		y = y or 0
		z = z or 0
		matrixStack:Push()
		matrixStack:Translate(x, y, z)
		drawer()
		matrixStack:Pop()
	end
end

local DrawJiji = WrapDrawer(function()
	matrixStack:Scale(1 / 2, 1 / 10, 1 / 2)
	jiji:Draw(matrixStack, 0, 0)
end)

local DrawNori = WrapDrawer(function()
	matrixStack:Scale(1 / 2, 1 / 10, 1 / 2)
	matrixStack:RotateY(180)
	nori:Draw(matrixStack, 0, 0)
end)

local DrawBoard = WrapDrawer(function()
end)

local DrawRange = WrapDrawer(function()
	matrixStack:Scale(1 / 8, 1 / 8, 1 / 8)
	pointer:Draw(matrixStack, 0, 0)
end)

function MoveToBoard(grid)
	local numGrid = grid.GetNumGrid()
	local go = -numGrid / 2 + 0.5
	matrixStack:Translate(go, 0, -go)
end

return {
	GetMousePosInBoard = function(grid)
		local numGrid = grid.GetNumGrid()
		local pos = gridRenderer:GetMousePosInGrid()
		if not pos then return end
		local x = math.floor(pos.x + numGrid / 2)
		local y = math.floor(-pos.y + numGrid / 2)
		print(string.format("pos %f %f => %d %d", pos.x, pos.y, x, y))
		if not grid.IsValidPos(x, y) then
			print(string.format("invalid pos %d %d", x, y))
			return
		end
		return {x = x, y = y}
	end,
	Draw2D = function() end,
	Draw3D = function(grid, pathGrid)
		LookAt(Vec3(6, 2, -8), Vec3(0, 0, 0))
		gridRenderer:Draw()
		local numGrid = grid.GetNumGrid()
		matrixStack:Push()
		MoveToBoard(grid)
		for x, y in gridTools.GridForeach(numGrid) do
			if grid[y][x] == 0 then
				DrawJiji(x, 0, -y)
			elseif grid[y][x] == 1 then
				DrawNori(x, 0, -y)
			elseif pathGrid and pathGrid[y][x] ~= -1 then
				DrawRange(x, 0, -y)
			else
				DrawBoard(x, 0, -y)
			end
		end
		matrixStack:Pop()
	end,
}
