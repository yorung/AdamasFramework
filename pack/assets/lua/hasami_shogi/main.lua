local gridTools = require("lua/hasami_shogi/grid_tools")
local commonTools = dofile("lua/hasami_shogi/common_tools.lua")
local renderer = dofile("lua/hasami_shogi/renderer3d.lua")
local ai = dofile("lua/hasami_shogi/ai.lua")

local sndAttack = Voice("sound/attack.wav")
local sndMove1 = Voice("sound/move1.wav")
local sndMove2 = Voice("sound/move2.wav")

PlayBgm("sound/background.mp3")

local numGrid = 9

local globalGrid = gridTools.CreateGrid(numGrid, function(x, y) return y == 0 and 1 or y == numGrid - 1 and 0 or -1 end)
local pathGrid

local function JudgeAndSound(grid, from, to)
	if gridTools.Judge(grid, from, to) > 0 then
		sndAttack:Stop()
		sndAttack:Play()
	else
		if math.random() < 0.5 then
			sndMove1:Stop()
			sndMove1:Play()
		else
			sndMove2:Stop()
			sndMove2:Play()
		end
	end
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
		local from, to, val = ai.FindBest(grid, myFaction, 0)
		if from ~= nil and to ~= nil then
			pathGrid = gridTools.FindPath(grid, from)
			print(string.format("move units from[%d %d] to[%d %d] val[%f]", from.x, from.y, to.x, to.y, val))
			Sleep(15)
			JudgeAndSound(grid, from, to)
			pathGrid = nil
		end
	end

	local function MoveUnit(grid, currentTurn)
		WaitClickLeft()
		local from = renderer.GetMousePosInBoard(grid)
		if not from then print("invalid pos") return end
		if grid[from.y][from.x] ~= currentTurn then
			print(string.format("my units not found at pos %d %d", from.x, from.y))
			return
		end
		print(string.format("my unit selected at pos %d %d", from.x, from.y))
		pathGrid = gridTools.FindPath(grid, from)
		WaitClickLeft()
		local to = renderer.GetMousePosInBoard(grid)
		if not to then
			print("invalid pos to move! reason: out of board")
			pathGrid = nil
			return
		end
		if pathGrid[to.y][to.x] == -1 then
			print(string.format("invalid pos to move! %d %d reason: pathGrid", to.x, to.y))
			pathGrid = nil
			return
		end
		print(string.format("move units from %d %d to %d %d", from.x, from.y, to.x, to.y))
		pathGrid = nil
		if grid[to.y][to.x] >= 0 then
			print("grid occupied")
			return
		end
		JudgeAndSound(grid, from, to)
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
	renderer.Draw2D(globalGrid, pathGrid)
end

function Draw3D()
	renderer.Draw3D(globalGrid, pathGrid)
end


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
