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

local function FindPath(pathGrid, grid, from)
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

return {
	CreateGrid = CreateGrid,
	FindPath = FindPath,
}
