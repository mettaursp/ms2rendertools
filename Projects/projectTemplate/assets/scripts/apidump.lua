local api = {
	Enums = {},
	Types = {},
	Objects = {}
}

print("=================")
print("running API dump")
print("=================")
print""
print""

for i,v in pairs(Meta) do
	local data = {
		Description = v.Description:gsub("\n","\\n"):gsub("\r","\\r"):gsub("\t","\\t"),
		Members = {},
		Functions = {}
	}
	
	if v.Parent then
		data.Inherits = v.Parent.Name
	end
	
	for a,c in pairs(v.Members) do
		local name = c.Type.Name
		
		if c.Type.IsEnum then
			local enum = name:match("Enum%.(.+)")
			api.Enums[enum] = api.Enums[enum] or {}
		end
		
		data.Members[a] = {
			Type = c.Type.Name,
			IsStatic = c.IsStatic,
			ReadOnly = c.ReadOnly,
			Description = c.Description:gsub("\n","\\n"):gsub("\r","\\r"):gsub("\t","\\t"),
			InheritedFrom = c.Parent.Name
		}
	end
	
	for a,c in pairs(v.Functions) do
		local func = {}
		
		for b,n in pairs(c) do
			local parameters = {}
			
			func[b] = {
				IsStatic = n.IsStatic,
				Parameters = parameters,
				InheritedFrom = n.ParentType,
				Description = n.Description:gsub("\n","\\n"):gsub("\r","\\r"):gsub("\t","\\t"),
				ReturnType = n.ReturnType,
				ReturnValue = n.ReturnValueDescription
			}
			
			for k,l in pairs(n.Parameters) do
				local name = l.Type.Name
				
				if l.Type.IsEnum then
					local enum = name:match("Enum%.(.+)")
					api.Enums[enum] = api.Enums[enum] or {}
				end
				
				parameters[k] = {
					Name = l.Name,
					Description = l.Description:gsub("\n","\\n"):gsub("\r","\\r"):gsub("\t","\\t"),
					Type = name,
					DefaultValue = l.DefaultValue
				}
			end
			
			if #parameters == 0 then
				parameters[0] = true
			end
		end
			
		if #func == 0 then
			func[0] = true
		end
		
		data.Functions[a] = func
	end
	
	if v.IsObject then
		api.Objects[v.Name] = data
	elseif v.IsClass then
		api.Types[v.Name] = data
	end
end

for i,v in pairs(api.Enums) do
	local enum = Enum[i]
	
	for a,c in pairs(enum) do
		table.insert(api.Enums[i], {
			Name = a,
			Value = c.Value
		})
	end
end

json.encode(api, "./assets/json/apidump.json")

print("=================")
print("finished dump")
print("=================")

return 0