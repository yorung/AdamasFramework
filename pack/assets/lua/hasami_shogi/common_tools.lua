local function DeepCopy(o)
	if type(o) ~= 'table' then return o end
	local t = {}
	for k, v in pairs(o) do t[k] = DeepCopy(v) end
	return t
end

local function DumpInternal(o)
	if type(o) ~= 'table' then return tostring(o) end
	local s = "{ "
	for k, v in pairs(o) do s = s .. string.format("%s:%s ", tostring(k), DumpInternal(v)) end
	return s.."}"
end

local function Dump(o)
	print(DumpInternal(o))
end

local function CountElements(o)
	local n = 0
	for k, v in pairs(o) do
		n = n + 1
	end
	return n
end

return {
	DeepCopy = DeepCopy,
	Dump = Dump,
	CountElements = CountElements,
}
