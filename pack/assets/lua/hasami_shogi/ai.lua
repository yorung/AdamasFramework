local gridTools = require("lua/hasami_shogi/grid_tools")

local function Evaluate(grid, myFaction)
	local myCnt = grid.Count(myFaction)
	local eneCnt = grid.Count(1 - myFaction)
	return myCnt - eneCnt + math.random() * 0.1
end

local function FindBest(grid, myFaction, depth)
	local maxVal = -10000
	local maxFrom
	local maxTo
	for from in gridTools.ValForeach(grid, function(v) return v == myFaction end) do
		local pathGrid = gridTools.FindPath(grid, from)
		for to in gridTools.ValForeach(pathGrid, function(v) return v ~= -1 end) do
			local gridTmp = gridTools.DuplicateGrid(grid)
			gridTools.Judge(gridTmp, from, to)
			local val
			if depth < 1 then
				val = -FindBest(gridTmp, 1 - myFaction, depth + 1)
			else
				val = Evaluate(gridTmp, myFaction)
			end

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
	return maxVal, maxFrom, maxTo
end

return {
	FindBest = function(grid, myFaction)
		return FindBest(grid, myFaction, 0)
	end,
}
