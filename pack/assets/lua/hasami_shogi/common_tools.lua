local function DeepCopy(o)
	if type(o) ~= 'table' then return o end
	local t = {}
	for k, v in pairs(o) do t[k] = DeepCopy(v) end
	return t
end

return {
	DeepCopy = DeepCopy
}
