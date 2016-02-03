local gridTools = dofile("lua/hasami_shogi/grid_tools.lua")
local commonTools = dofile("lua/hasami_shogi/common_tools.lua")

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
		pathGrid = gridTools.FindPath(grid, numGrid, from)
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
		gridTools.Judge(grid, from, to, currentTurn)
		return true
	end

	local function Think(currentTurn)
		local maxVal = -1
		local maxFrom
		local maxTo
		for from in gridTools.ValForeach(grid, numGrid, function(v) return v == currentTurn end) do
			pathGrid = gridTools.FindPath(grid, numGrid, from)
--			Sleep(3)
			for to in gridTools.ValForeach(pathGrid, numGrid, function(v) return v ~= -1 end) do
				local gridTmp = commonTools.DeepCopy(grid)
				gridTools.Judge(gridTmp, from, to, currentTurn)
				local val = math.random()
				if maxVal < val then
					maxVal = val
					maxFrom = from
					maxTo = to
				end
			end
			pathGrid = nil
		end
		if maxFrom ~= nil and maxTo ~= nil then
			print("maxFrom", maxFrom)
			print("maxTo", maxTo)
			pathGrid = gridTools.FindPath(grid, numGrid, maxFrom)
			Sleep(15)
			gridTools.Judge(grid, maxFrom, maxTo, currentTurn)
			pathGrid = nil
		end
	end

	while true do
		while not MoveUnit(0) do end
		Think(1)
	--	while not MoveUnit(1) do end
	end
end)

function Update()
	assert(coroutine.resume(co))
end

function Draw2D()
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
end

function Draw3D()
end

LoadSkyBox("hakodate.jpg", "sky_photosphere")
