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
			return -1
		end,
	}
	for y = 0, numGrid - 1 do
		_[y] = {}
		for x = 0, numGrid - 1 do _[y][x] = valFunc(x, y) end
	end
	return _
end

local function FindPath(grid, numGrid, from)
	local pathGrid = CreateGrid(numGrid, function(x, y) return -1 end)
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

local function ValForeach(grid, numGrid, func)
	local gen = GridForeach(numGrid)
	return function()
		while true do
			local x, y = gen()
			if x == nil then
				return
			end
			if func(grid[y][x]) then
				return {x = x, y = y}
			end
		end
	end
end

local function Judge(grid, from, to, currentTurn)
	grid[to.y][to.x] = grid[from.y][from.x]
	grid[from.y][from.x] = -1

	local function DetectToward(dx, dy)
		local x = to.x + dx
		local y = to.y + dy
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
		local x = to.x + dx
		local y = to.y + dy
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

return {
	CreateGrid = CreateGrid,
	FindPath = FindPath,
	GridForeach = GridForeach,
	ValForeach = ValForeach,
	Judge = Judge,
}
