local gridTools = require("lua/hasami_shogi/grid_tools")

local abCnt = 0

local function Evaluate(grid, myFaction)
	local myCnt = grid.Count(myFaction)
	local eneCnt = grid.Count(1 - myFaction)
	return myCnt - eneCnt + math.random() * 0.1
end

local function FindBest(grid, myFaction, depth, a, b)
	local maxFrom
	local maxTo
	local maxVal = -10000
	local evaluator
	if depth < 1 then
		local enemyFaction = 1 - myFaction
		evaluator = function(gridForEval) return -FindBest(gridForEval, enemyFaction, depth + 1, -b, -a) end
	else
		evaluator = function(gridForEval) return Evaluate(gridForEval, myFaction) end
	end

	for from in gridTools.ValForeach(grid, function(v) return v == myFaction end) do
		local pathGrid = gridTools.FindPath(grid, from)
		for to in gridTools.ValForeach(pathGrid, function(v) return v ~= -1 end) do
			local gridTmp = gridTools.DuplicateGrid(grid)
			gridTools.Judge(gridTmp, from, to)
			local val = evaluator(gridTmp)
			if depth == 0 then
				print(string.format("depth[%d] from[%d %d] to[%d %d] val[%f] a[%f] b[%f]", depth, from.x, from.y, to.x, to.y, val, a, b))
				coroutine.yield()
			end
			if maxVal < val then
				maxVal = val
				maxFrom = from
				maxTo = to
			end
			if a < val then
				a = val
				assert(maxVal == a)
				if a >= b then
					abCnt = abCnt + 1
					return maxVal, maxFrom, maxTo
				end
			end
		end
	end
--	assert(maxVal == a)
	return maxVal, maxFrom, maxTo
end

return {
	FindBest = function(grid, myFaction)
		abCnt = 0
		local val, from, to = FindBest(grid, myFaction, 0, -10000, 10000)
		print(string.format("%d alpha-beta pruning proceeded", abCnt))
		return val, from, to
	end,
}
