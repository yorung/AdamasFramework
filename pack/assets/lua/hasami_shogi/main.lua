local gridTools = dofile("lua/hasami_shogi/grid_tools.lua")

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

local numGrid = 9


local grid = gridTools.CreateGrid(numGrid, function(x, y) return y == 0 and 1 or y == numGrid - 1 and 0 or -1 end)
local pathGrid

local smallerSize = math.min(SCR_W, SCR_H)
local function MoveToBoard()
	matrixStack:Translate(SCR_W / 2, SCR_H / 2, 0)
	matrixStack:Scale(smallerSize, smallerSize, 1)
	matrixStack:Translate(-0.5, -0.5, 0)
end

local boardLT = {x = SCR_W / 2 - smallerSize * 0.5, y = SCR_H / 2 - smallerSize * 0.5}
local boardRB = {x = SCR_W / 2 + smallerSize * 0.5, y = SCR_H / 2 + smallerSize * 0.5}

local function GetMousePosInBoard()
	local p = GetMousePos()
	local x = math.floor((p.x - boardLT.x) / (boardRB.x - boardLT.x) * numGrid)
	local y = math.floor((p.y - boardLT.y) / (boardRB.y - boardLT.y) * numGrid)
	if not grid.IsValidPos(x, y) then
		print(string.format("invalid pos %d %d", x, y))
		return
	end
	return {x = x, y = y}
end

local function Detection(pos, currentTurn)
	local function DetectToward(dx, dy)
		local x = pos.x + dx
		local y = pos.y + dy
		while true do
			local t = grid.GetGridSafe(x, y)
			if t == currentTurn then
				return true
			elseif t < 0 then
				return false
			end
			x = x + dx
			y = y + dy
		end
	end
	local function TryKill(dx, dy)
		if not DetectToward(dx, dy) then return end
		local x = pos.x + dx
		local y = pos.y + dy
		while true do
			local t = grid.GetGridSafe(x, y)
			if t == currentTurn then
				return
			end
			grid[y][x] = -1
			x = x + dx
			y = y + dy
		end
	end
	TryKill(1, 0)
	TryKill(-1, 0)
	TryKill(0, 1)
	TryKill(0, -1)
end

local co = coroutine.create(function()
	local function Sleep(f) for i=1, f do coroutine.yield() end end
	local function WaitClickLeft()
		Sleep(1)
		while GetKeyCount(1) ~= 1 do Sleep(1) end
	end
	local function MoveUnit(currentTurn)
		WaitClickLeft()
		local from = GetMousePosInBoard()
		if not from then print("invalid pos") return end
		if grid[from.y][from.x] ~= currentTurn then
			print(string.format("my units not found at pos %d %d", from.x, from.y))
			return
		end
		pathGrid = gridTools.FindPath(gridTools.CreateGrid(numGrid, function(x, y) return -1 end), grid, from)
		WaitClickLeft()
		local to = GetMousePosInBoard()
		if not to then
			print("invalid pos to move! reason: out of board")
			pathGrid = nil
			return
		end
		if pathGrid[to.y][to.x] == -1 then
			print("invalid pos to move! reason: pathGrid")
			pathGrid = nil
			return
		end
		print(string.format("move units from %d %d to %d %d", from.x, from.y, to.x, to.y))
		pathGrid = nil
		if grid[to.y][to.x] >= 0 then
			print("grid occupied")
			return
		end
		grid[to.y][to.x] = grid[from.y][from.x]
		grid[from.y][from.x] = -1
		Detection(to, currentTurn)
		return true
	end

	while true do
		while not MoveUnit(0) do end
		while not MoveUnit(1) do end
	end
end)

function Update()
	assert(coroutine.resume(co))
end

function Draw2D()
	matrixStack:Push()
	MoveToBoard()
	matrixStack:Scale(1 / numGrid, 1 / numGrid, 1)
	for x = 0, numGrid - 1 do
		for y = 0, numGrid - 1 do
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
	end
	matrixStack:Pop()
end

function Draw3D()
end

LoadSkyBox("hakodate.jpg", "sky_photosphere")