local gridTools = require("lua/hasami_shogi/grid_tools")

LoadSkyBox("hakodate.jpg", "sky_photosphere")

local matrixStack = MatrixStack()

local jiji = Image("jiji.dds")
jiji:SetCell(0, {left = 0, top = 0, right = 256, bottom = 256})

local board = Image("delaymap.png")
board:SetCell(0, {left = 0, top = 0, right = 64, bottom = 64})

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
	matrixStack:Scale(1 / 256, 1 / 256, 1)
	jiji:DrawCell(matrixStack, 0)
end)

local DrawReverseJiji = WrapDrawer(function()
	matrixStack:Translate(1, 1, 0)
	matrixStack:Scale(1 / 256, 1 / 256, 1)
	matrixStack:RotateZ(180)
	jiji:DrawCell(matrixStack, 0)
end)

local DrawBoard = WrapDrawer(function()
	matrixStack:Scale(1 / 64, 1 / 64, 1)
	board:DrawCell(matrixStack, 0)
end)

local DrawRange = WrapDrawer(function()
	matrixStack:Translate(1, 1, 0)
	matrixStack:Scale(1 / 64, 1 / 64, 1)
	matrixStack:RotateZ(180)
	board:DrawCell(matrixStack, 0)
end)

--[[
local chips = {}
for i = 0, 15 do
	local img = Image(string.format("chip/chip%02d.png", i))
	img:SetCell(0, {left = 0, top = 0, right = 64, bottom = 64})
	chips[i] = {
		Draw = WrapDrawer(function()
			matrixStack:Scale(1 / 64, 1 / 64, 1)
			img:DrawCell(matrixStack, 0)
		end)
	}
end]]

local smallerSize = math.min(SCR_W, SCR_H)
local function MoveToBoard()
	matrixStack:Translate(SCR_W / 2, SCR_H / 2, 0)
	matrixStack:Scale(smallerSize, smallerSize, 1)
	matrixStack:Translate(-0.5, -0.5, 0)
end

local boardLT = {x = SCR_W / 2 - smallerSize * 0.5, y = SCR_H / 2 - smallerSize * 0.5}
local boardRB = {x = SCR_W / 2 + smallerSize * 0.5, y = SCR_H / 2 + smallerSize * 0.5}

return {
	GetMousePosInBoard = function(grid)
		local numGrid = grid.GetNumGrid()
		local p = GetMousePos()
		local x = math.floor((p.x - boardLT.x) / (boardRB.x - boardLT.x) * numGrid)
		local y = math.floor((p.y - boardLT.y) / (boardRB.y - boardLT.y) * numGrid)
		if not grid.IsValidPos(x, y) then
			print(string.format("invalid pos %d %d", x, y))
			return
		end
		return {x = x, y = y}
	end,
	Draw2D = function(grid, pathGrid)
		local numGrid = grid.GetNumGrid()
		matrixStack:Push()
		MoveToBoard()
		matrixStack:Scale(1 / numGrid, 1 / numGrid, 1)
		for x, y in gridTools.GridForeach(numGrid) do
			if grid[y][x] == 0 then
				DrawJiji(x, y, 2)
			elseif grid[y][x] == 1 then
				DrawReverseJiji(x, y, 2)
			elseif pathGrid and pathGrid[y][x] ~= -1 then
				DrawRange(x, y, 1)
			else
				DrawBoard(x, y, 1)
			end
		end
		matrixStack:Pop()
	end,
}