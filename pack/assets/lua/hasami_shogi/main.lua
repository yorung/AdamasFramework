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


local globalGrid = gridTools.CreateGrid(numGrid, function(x, y) return y == 0 and 1 or y == numGrid - 1 and 0 or -1 end)
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
	if not globalGrid.IsValidPos(x, y) then
		print(string.format("invalid pos %d %d", x, y))
		return
	end
	return {x = x, y = y}
end

local function FindBest(grid, myFaction, evaluator, depth)
	local maxVal = -10000
	local maxFrom
	local maxTo
--	print(string.format("FindBest numGrid[%d] myFaction[%d] depth[%d]", numGrid, myFaction, depth))
	for from in gridTools.ValForeach(grid, function(v) return v == myFaction end) do
		local pathGrid = gridTools.FindPath(grid, from)
		for to in gridTools.ValForeach(pathGrid, function(v) return v ~= -1 end) do
			local gridTmp = gridTools.DuplicateGrid(grid)
			gridTools.Judge(gridTmp, from, to)
			local val = evaluator(gridTmp, myFaction, depth + 1)
			if depth == 0 then
				print(string.format("depth[%d] from[%d %d] to[%d %d] val[%f]", depth, from.x, from.y, to.x, to.y, val))
			--[[	if from.x == 2 and from.y == 0 and to.x == 2 and to.y == 1 then
					print(string.format("Dump for FindBest evaluator numGrid[%d] myFaction[%d] depth[%d]", numGrid, myFaction, depth))
					commonTools.Dump(gridTmp)
				end]]
			end
			if maxVal < val then
				maxVal = val
				maxFrom = from
				maxTo = to
			end
		end
	end
	return maxFrom, maxTo, maxVal
end

local function Evaluate(grid, myFaction, depth)
--	print(string.format("Evaluate numGrid[%d] myFaction[%d] depth[%d]", numGrid, myFaction, depth))
	if depth < 2 then
		local enemyFaction = 1 - myFaction
		local from, to, val = FindBest(grid, enemyFaction, Evaluate, depth)
		return -val
	end
	local myCnt = grid.Count(myFaction)
	local eneCnt = grid.Count(1 - myFaction)
	return myCnt - eneCnt + math.random() * 0.1
end

local co = coroutine.create(function()
	local function Sleep(f) for i=1, f do coroutine.yield() end end
	local function WaitClickLeft()
		Sleep(1)
		while GetKeyCount(1) ~= 1 do Sleep(1) end
	end

	local function Think(grid, myFaction)
		Sleep(1)
	--	print(string.format("Think numGrid[%d] myFaction[%d]", numGrid, myFaction))
		local from, to, val = FindBest(grid, myFaction, Evaluate, 0)
		if from ~= nil and to ~= nil then
			pathGrid = gridTools.FindPath(grid, from)
			print(string.format("move units from[%d %d] to[%d %d] val[%f]", from.x, from.y, to.x, to.y, val))
			Sleep(15)
			gridTools.Judge(grid, from, to)
			pathGrid = nil
		end
	end

	local function MoveUnit(grid, currentTurn)
		WaitClickLeft()
		local from = GetMousePosInBoard()
		if not from then print("invalid pos") return end
		if grid[from.y][from.x] ~= currentTurn then
			print(string.format("my units not found at pos %d %d", from.x, from.y))
			return
		end
		pathGrid = gridTools.FindPath(grid, from)
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
		gridTools.Judge(grid, from, to)
		return true
	end


	while true do
		while not MoveUnit(globalGrid, 0) do end
--		while not MoveUnit(globalGrid, 1) do end
		Think(globalGrid, 1)
	end
end)

function Update()
	local globalCnt = commonTools.CountElements(_G)
	assert(coroutine.resume(co))
	assert(globalCnt == commonTools.CountElements(_G))
--[[	if GetKeyCount(2) == 1 then
		for k, v in pairs(_G) do
			print(string.format("%s:%s", k, v))
		end
	end]]
end

function Draw2D()
	matrixStack:Push()
	MoveToBoard()
	matrixStack:Scale(1 / numGrid, 1 / numGrid, 1)
	for x, y in gridTools.GridForeach(numGrid) do
		if globalGrid[y][x] == 0 then
			DrawJiji(x, y, 2)
		elseif globalGrid[y][x] == 1 then
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

--[[
local tbl = {
{-1, 1, 1, 0},
{-1, 1,-1,-1},
{-1,-1,-1,-1},
{ 0, 0, 0, 0},
}
local t = gridTools.CreateGrid(4, function(x, y) return tbl[y + 1][x + 1] end)



--Think(t, 4, 1)
local from, to, val = FindBest(t, 4, 1, Evaluate, 0)
--local from, to, val = FindBest(t, 4, 0, Evaluate, 1)
print(string.format("move units from[%d %d] to[%d %d] val[%f]", from.x, from.y, to.x, to.y, val))
]]
