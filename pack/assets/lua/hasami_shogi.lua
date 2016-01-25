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
	jiji:DrawCell(0)
end)

local DrawReverseJiji = WrapDrawer(function()
	matrixStack:Translate(1, 1, 0)
	matrixStack:Scale(1 / 256, 1 / 256, 1)
	matrixStack:RotateZ(180)
	jiji:DrawCell(0)
end)

local DrawBoard = WrapDrawer(function()
	matrixStack:Scale(1 / 64, 1 / 64, 1)
	board:DrawCell(0)
end)

--[[
local chips = {}
for i = 0, 15 do
	local img = Image(string.format("chip/chip%02d.png", i))
	img:SetCell(0, {left = 0, top = 0, right = 64, bottom = 64})
	chips[i] = {
		Draw = WrapDrawer(function()
			matrixStack:Scale(1 / 64, 1 / 64, 1)
			img:DrawCell(0)
		end)
	}
end]]

local numGrid = 9

local grid = {}
for y = 0, numGrid - 1 do
	grid[y] = {}
	for x = 0, numGrid - 1 do
		if y == 0 then
			grid[y][x] = 1
		elseif y == numGrid - 1 then
			grid[y][x] = 0
		else
			grid[y][x] = -1
		end
	end
end

local smallerSize = math.min(SCR_W, SCR_H)
local function MoveToBoard()
	matrixStack:Translate(SCR_W / 2, SCR_H / 2, 0)
	matrixStack:Scale(smallerSize, smallerSize, 1)
	matrixStack:Translate(-0.5, -0.5, 0)
end

local boardLT = {x = SCR_W / 2 - smallerSize * 0.5, y = SCR_H / 2 - smallerSize * 0.5}
local boardRB = {x = SCR_W / 2 + smallerSize * 0.5, y = SCR_H / 2 + smallerSize * 0.5}
--[[
matrixStack:Push()
	MoveToBoard()
	local boardLT = GetScreenPos()
	matrixStack:Translate(numGrid, numGrid, 0)
local boardRB = GetScreenPos()
matrixStack:Pop()
print(string.format("lt = %d %d rb = %d %d", boardLT.x, boardLT.y, boardRB.x, boardRB.y))
]]

local function IsValidPos(x, y)
	return x >= 0 and y >= 0 and x < numGrid and y < numGrid
end

local function GetMousePosInBoard()
	local p = GetMousePos()
	local x = math.floor((p.x - boardLT.x) / (boardRB.x - boardLT.x) * numGrid)
	local y = math.floor((p.y - boardLT.y) / (boardRB.y - boardLT.y) * numGrid)
	if not IsValidPos(x, y) then
		print(string.format("invalid pos %d %d", x, y))
		return
	end
	return {x = x, y = y}
end

local function GetGridSafe(x, y)
	if IsValidPos(x, y) then
		return grid[y][x]
	end
	return -1
end

local function Detection(pos, currentTurn)
	local function DetectToward(dx, dy)
		local x = pos.x + dx
		local y = pos.y + dy
		while true do
			local t = GetGridSafe(x, y)
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
			local t = GetGridSafe(x, y)
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
		while GetKeyCount(1) ~= 1 do Sleep(1) end
	end
	local function MoveUnit(currentTurn)
		Sleep(1) -- prevent infinity loop
		WaitClickLeft()
		local from = GetMousePosInBoard()
		if not from then
			print("invalid pos")
			return
		end
		if grid[from.y][from.x] ~= currentTurn then
			print(string.format("my units not found at pos %d %d", from.x, from.y))
			return
		end
		Sleep(1)
		WaitClickLeft()
		local to = GetMousePosInBoard()
		if not to then
			print("invalid pos to move")
			return
		end
		print(string.format("move units from %d %d to %d %d", from.x, from.y, to.x, to.y))
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
		--	DrawJiji(x, y, 1)
			if grid[y][x] == 0 then
				DrawJiji(x, y, 2)
			elseif grid[y][x] == 1 then
				DrawReverseJiji(x, y, 2)
			else
			--	chips[x % 3].Draw(x, y, 1)
				DrawBoard(x, y, 1)
			end
		end
	end

--	matrixStack:Scale(2 / 256, 2 / 256, 1)
--	jiji:DrawCell(0)
	matrixStack:Pop()
end

function Draw3D()
end

LoadSkyBox("hakodate.jpg", "sky_photosphere")
