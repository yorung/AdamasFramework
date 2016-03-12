local gridTools = require("lua/hasami_shogi/grid_tools")

local Evaluate

local function FindBest(grid, myFaction, depth)
	local maxVal = -10000
	local maxFrom
	local maxTo
	for from in gridTools.ValForeach(grid, function(v) return v == myFaction end) do
		local pathGrid = gridTools.FindPath(grid, from)
		for to in gridTools.ValForeach(pathGrid, function(v) return v ~= -1 end) do
			local gridTmp = gridTools.DuplicateGrid(grid)
			gridTools.Judge(gridTmp, from, to)
			local val = Evaluate(gridTmp, myFaction, depth + 1)
			if depth == 0 then
				print(string.format("depth[%d] from[%d %d] to[%d %d] val[%f]", depth, from.x, from.y, to.x, to.y, val))
				coroutine.yield()
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

Evaluate = function(grid, myFaction, depth)
--	print(string.format("Evaluate numGrid[%d] myFaction[%d] depth[%d]", numGrid, myFaction, depth))
	if depth < 2 then
		local enemyFaction = 1 - myFaction
		local from, to, val = FindBest(grid, enemyFaction, depth)
		return -val
	end
	local myCnt = grid.Count(myFaction)
	local eneCnt = grid.Count(1 - myFaction)
	return myCnt - eneCnt + math.random() * 0.1
end

return {
	FindBest = FindBest
}
