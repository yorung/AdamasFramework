local wall = -2

local function GridForeach(numGrid)
	local i = 0
	local sq = numGrid * numGrid
	return function()
		if i >= sq then
			return nil
		end
		local x = i % numGrid
		local y = math.floor(i / numGrid)
		i = i + 1
		return x, y
	end
end

local function CreateGrid(numGrid, valFunc)
	local function IsValidPos(x, y)
		return x >= 0 and y >= 0 and x < numGrid and y < numGrid
	end

	local _
	_ = {
		IsValidPos = IsValidPos,
		GetGridSafe = function(x, y)
			if IsValidPos(x, y) then
				return _[y][x]
			end
			return wall
		end,
		Count = function(faction)
			local cnt = 0
			for x, y in GridForeach(numGrid) do
				if faction == _[y][x] then
					cnt = cnt + 1
				end
			end
			return cnt
		end,
		GetNumGrid = function() return numGrid end,
	}
	for y = 0, numGrid - 1 do
		_[y] = {}
		for x = 0, numGrid - 1 do _[y][x] = valFunc(x, y) end
	end
	return _
end

local function DuplicateGrid(grid)
	return CreateGrid(grid.GetNumGrid(), function(x, y) return grid[y][x] end)
end

local function FindPath(grid, from)
	local pathGrid = CreateGrid(grid.GetNumGrid(), function(x, y) return -1 end)
	local function findDir(dir)
		local x = from.x
		local y = from.y
		while true do
			x = x + dir.x
			y = y + dir.y
			if not grid.IsValidPos(x, y) then
				return
			end
			if grid[y][x] ~= -1 then
				return
			end
			pathGrid[y][x] = 0
		end
	end
	findDir({x = 1, y = 0})
	findDir({x = -1, y = 0})
	findDir({x = 0, y = 1})
	findDir({x = 0, y = -1})
	return pathGrid
end

local function ValForeach(grid, func)
--	print(string.format("ValForeach numGrid[%d]", numGrid))
	local gen = GridForeach(grid.GetNumGrid())
	return function()
		while true do
			local x, y = gen()
			if x == nil then
				return
			end
--			print(string.format("ValForeach x[%d] y[%d]", x, y))
			if func(grid[y][x]) then
				return {x = x, y = y}
			end
		end
	end
end

local function Judge(grid, from, to)
	local myFaction = grid[from.y][from.x]
	grid[to.y][to.x] = myFaction
	grid[from.y][from.x] = -1
	local enemyFaction = 1 - myFaction
	local function TryFlank(x, y, dx, dy)
		x = x + dx
		y = y + dy
		local t = grid.GetGridSafe(x, y)
		if t == myFaction then return true end
		if t == enemyFaction and TryFlank(x, y, dx, dy) then
			grid[y][x] = -1
			return true
		end
	end
	local function IsSurrounded(x, y)
		local t = grid.GetGridSafe(x, y)
		if t == -1 then return false end
		if t == wall or t == myFaction then return true end
		assert(t == enemyFaction)
		grid[y][x] = wall
		local surrounded = IsSurrounded(x + 1, y) and IsSurrounded(x - 1, y) and IsSurrounded(x, y + 1) and IsSurrounded(x, y - 1)
		grid[y][x] = enemyFaction
		return surrounded
	end
	local function KillSurroundTroop(x, y)
		local t = grid.GetGridSafe(x, y)
		if t == enemyFaction then
			grid[y][x] = -1
			KillSurroundTroop(x + 1, y)
			KillSurroundTroop(x - 1, y)
			KillSurroundTroop(x, y + 1)
			KillSurroundTroop(x, y - 1)
		end
	end
	local function TryKill(dx, dy)
		TryFlank(to.x, to.y, dx, dy)
		if IsSurrounded(to.x + dx, to.y + dy) then
			KillSurroundTroop(to.x + dx, to.y + dy)
		end
	end
	TryKill(1, 0)
	TryKill(-1, 0)
	TryKill(0, 1)
	TryKill(0, -1)
end

return {
	CreateGrid = CreateGrid,
	FindPath = FindPath,
	GridForeach = GridForeach,
	ValForeach = ValForeach,
	Judge = Judge,
	Count = Count,
	DuplicateGrid = DuplicateGrid,
}
