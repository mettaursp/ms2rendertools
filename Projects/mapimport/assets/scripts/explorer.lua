function create(type, parent, properties)
	local object = GameObject(type)
	
	for name, value in pairs(properties) do
		object[name] = value
	end
	
	object.Parent = parent
	
	return object
end

local createStructure

function createStructure(meta, parent)
	local object = create(meta.Class, parent, meta.Properties)
	
	for i,child in pairs(meta.Children) do
		createStructure(child, object)
	end
	
	return object
end

local types = {
	string = function(data)
		return data
	end,
	float = tonumber,
	double = tonumber,
	int = tonumber,
	long = tonumber,
	short = tonumber,
	char = tonumber,
}

function makeData(meta, data)
	local dataType = type(data)
	
	if dataType == "string" then
		--[[print("type", meta.Name)
		for i,v in pairs(meta) do
			print("", i,v)
		end]]
		
		if meta.IsEnum then
			return Enum[meta.Name:sub(6)][data]
		end
		
		return types[meta.Name](data)
	else
		--print(dataType, data)
		
		local args = {}
		
		if #data ~= 0 then
			for i, value in ipairs(data) do
				--print("\tarray", i, value)
				for i,v in pairs(meta.Functions[meta.Name]) do print(i,v) end
				
				args[i] = makeData(value)
			end
		end
		
		local value = _G[meta.Name](table.unpack(args))
		
		if #data == 0 then
			for name, innerData in pairs(data) do
				--print("inner: ",name, innerData)
				value[name] = makeData(meta.Members[name].Type, innerData)
			end
		end
		
		return value
	end
end

function createInstance(objectType, parent, properties)
	local object = GameObject(objectType)
	local members = Meta[objectType].Members
	
	--print(objectType)
	
	for name, value in pairs(properties) do
		--print("\tproperty", name, value)
		object[name] = makeData(members[name].Type, value)
	end
	
	object.Parent = parent
	
	return object
end

local createArchetype

function createArchetype(meta, parent)
	local object = createInstance(meta.Class, parent, meta.Properties)
	
	for i,child in pairs(meta.Children) do
		createArchetype(child, object)
	end
	
	return object
end

local archetypes = {
	Explorer = json.decode("./assets/json/explorer.json", true)
}
local empty = {}

function archetype(namespace, name)
	if not (archetypes[namespace] or empty)[name] then
		error("attempt to create instance of nonexistant archetype '"..(namespace or "").."."..(name or "").."'")
	end
	
	return createArchetype(archetypes[namespace][name])
end

function initialize(screen)
	wait()
	--[[local button = createStructure {
		Class = "DeviceTransform",
		Properties = {
			Name = "ButtonTransform",
			Size = DeviceVector(0, 200, 0, 200),
			Position = DeviceVector(0.5, 0, 0.5, 0)
		},
		Children = {
			
		}
	}]]
	
	--local screen2 = 
	
	local button = archetype("Explorer", "Button")
	button.ScreenCanvas.Button.Hover = button.ScreenCanvas.Button.HoverAppearance
	button.ScreenCanvas.Button.Idle = button.ScreenCanvas.Button.IdleAppearance
	button.ScreenCanvas.Button.Pressed = button.ScreenCanvas.Button.PressedAppearance
	button.ScreenCanvas.Button.Binding = button.InputSubscriber:Subscribe(Engine.GameWindow.UserInput:GetInput(Enum.InputCode.MouseLeft))
	button.Parent = screen

	--[[print(button.Parent, button, button:GetChildren())
	for i=1,button:GetChildren() do
		local v = button:Get(i-1)
		print("",i,v, v ~= nil and v:GetChildren())
		for a=1,v:GetChildren() do
			local c = v:Get(a-1)
			print("\t",a,c, c:GetChildren())
		end
	end]]
end

return coroutine.wrap(function(screen)
	print(pcall(initialize, screen))
end)