print("abc")
	print("hi am thread",coroutine.running())

local resolution = GameObject.FrameBuffer.WindowSize

local meshes = GameObject("Object")
meshes.Name = "Meshes"
meshes:SetParent(Engine)

local assets
print("pcall", pcall(function() assets = json.decode("./assets/json/assets.json", true) end))

for name, path in pairs(assets.meshes) do
	print(name, path)
	GameObject.MeshLoader.NewAsset(name, path):SetParent(meshes)
	print("loaded", name)
end

GameObject.MeshLoader.NewAsset("WireSphere", "assets/cs300models/sphere.obj", Enum.VertexMode.Wireframe):SetParent(meshes)

local textures = GameObject("Textures")
textures.Name = "Textures"
textures:SetParent(Engine)

for name, path in pairs(assets.textures) do
	textures:Add(textures:Create(path), name)
end

local fonts = GameObject("Fonts")
fonts:SetParent(Engine)

local sans = GameObject("Font")
sans.Name = "Sans"
sans:Load("assets/fonts/Sans", "Sans")
sans:SetParent(fonts)

local environments = GameObject("Environments")
environments:SetParent(Engine)

local level = GameObject("Environment")
level.Name = "Level"
level:SetParent(environments)

local simulation = GameObject("Simulation")
simulation:SetParent(level)

local aspectRatio = GameObject.FrameBuffer.WindowSize.Width / GameObject.FrameBuffer.WindowSize.Height
local size = 5 / 3
local defaultWidth = aspectRatio * size
local defaultHeight = size
local defaultProjection = 1
local defaultNear = 0.1
local defaultFar = 10000

local camera = GameObject("Camera")
camera:SetParent(level)
camera:SetProperties(defaultWidth, defaultHeight, defaultProjection, defaultNear, defaultFar)--120, resolution.Width / resolution.Height, 0.1, 10000)
camera:SetTransformation(Matrix3(0, 0, 10))

local light = GameObject("Light")
light.Enabled = true
light.Direction = Vector3(-0.5, -1, 0.5)
light.Brightness = 1.5
light.Diffuse = RGBA(0.3, 0.2, 0.4, 1)
light.Specular = RGBA(1, 1, 1, 1)
light.Ambient = RGBA(0.1, 0.1, 0.1, 1)
light.Type = Enum.LightType.Directional
light:SetParent(simulation)

local scene = GameObject("Scene")
scene:SetParent(level)
scene.CurrentCamera = camera
scene.GlobalLight = light

local materials = GameObject("Materials")
materials:SetParent(Engine)

local material = GameObject("Material")
material.Shininess = 250
material.Diffuse = RGBA(0.5, 0.5, 0.5, 0)
material.Specular = RGBA(1, 1, 1, 0)
material.Ambient = RGBA(0.5, 0.5, 0.5, 0)
material.Emission = RGBA(0, 0, 0, 0)
material:SetParent(Engine)

function files(directory)
	local fileList = {}
	
	local file, err = io.popen("dir "..directory.."\\*.ply /b")
	
	if not file then
		print(directory, err)
	end
	
	for file in file:lines() do
		fileList[file] = directory.."\\"..file
	end
	
	file:close()
	
	return fileList
end

function scanAndLoad(directory, parent)
	local file, err = io.popen([[dir "]]..directory..[[" /b /ad]])
	
	if not file then
		print(directory, err)
	end
	
	for dir in file:lines() do
		local subDirectory = directory.."\\"..dir
		
		local transform = GameObject("Transform")

		transform.Name = dir
		transform.Parent = parent
		
		for name, filePath in pairs(files(subDirectory)) do
			local mesh = GameObject.MeshLoader.NewAsset(name, filePath)
			
			local model = GameObject("Model")
			model.Asset = mesh
			model.Parent = transform
			model.MaterialProperties = material
			
			mesh.Parent = model
			
			scene:AddObject(model)
		end
		
		scanAndLoad(subDirectory, transform)
	end
	
	file:close()
end

local factoryTransform = GameObject("Transform")

factoryTransform.Name = "Factory"
factoryTransform.Parent = simulation
factoryTransform.Transformation = Matrix3.NewScale(0.0025, 0.0025, 0.0025)

print(pcall(function()
scanAndLoad(".\\assets\\cs350powerplant", factoryTransform)
end))

local lights = {}
local lightColors = {
	RGBA(1, 0, 0, 0.99),
	RGBA(1, 0.5, 0, 0.99),
	RGBA(1, 1, 0, 0.99),
	RGBA(0, 1, 0, 0.99),
	RGBA(0, 1, 1, 0.99),
	RGBA(0, 0, 1, 0.99),
	RGBA(0.5, 0, 1, 0.99),
	RGBA(1, 0, 1, 0.99)
}

for i = 1, 8 do
	local debugMaterial = GameObject("Material")
	debugMaterial.Shininess = 1
	debugMaterial.Diffuse = RGBA(0, 0, 0, 0)
	debugMaterial.Specular = RGBA(0, 0, 0, 0)
	debugMaterial.Emission = lightColors[i]
	
	local lightOrb = GameObject("Transform")
	lightOrb:SetParent(simulation)
	lightOrb.IsStatic = false
	lightOrb.Transformation = Matrix3(0, 0, 8) * Matrix3.PitchRotation(i * math.pi / 4) * Matrix3.NewScale(0.25, 0.25,0.25)
	lightOrb:Update(0)

	local lightOrbModel = GameObject("Model")
	lightOrbModel.Asset = Engine.Meshes.Sphere
	lightOrbModel:SetParent(lightOrb)
	lightOrbModel.Color = lightColors[i]
	lightOrbModel.GlowColor = lightColors[i]
	lightOrbModel.MaterialProperties = debugMaterial

	local testLight = GameObject("Light")
	testLight.Enabled = true
	testLight.Position = Vector3(0, 0, 0)
	testLight.Diffuse = lightColors[i]
	testLight.Specular = lightColors[i]
	testLight.Ambient = lightColors[i]
	testLight.Type = Enum.LightType.Point
	testLight.Attenuation = Vector3(1, 0, 0.025)
	testLight.Brightness = 1
	testLight:SetParent(lightOrb)
	
	scene:AddObject(lightOrbModel)
	scene:AddLight(testLight)
	
	lights[i] = lightOrb
	
	debugMaterial.Parent = lightOrbModel
end

local lightOrb = GameObject("Transform")
lightOrb.Parent = simulation
lightOrb.IsStatic = false
lightOrb.Transformation = Matrix3(0, 1000, 10) * Matrix3.PitchRotation(math.pi / 2) * Matrix3.NewScale(1, 1, 1)
lightOrb:Update(0)

local lightOrbModel = GameObject("Model")
lightOrbModel.Asset = Engine.Meshes.Sphere
lightOrbModel.Parent = lightOrb
lightOrbModel.Color = RGBA(1, 1, 1, 0.99)
lightOrbModel.GlowColor = RGBA(1, 1, 1, 0.99)

do
	local debugMaterial = GameObject("Material")
	debugMaterial.Shininess = 1
	debugMaterial.Diffuse = RGBA(0, 0, 0, 0)
	debugMaterial.Specular = RGBA(0, 0, 0, 0)
	debugMaterial.Emission = RGBA(1, 1, 1, 1)
	debugMaterial.Parent = lightOrbModel
	
	lightOrbModel.MaterialProperties = debugMaterial
end

local testLight = GameObject("Light")
testLight.Enabled = true
testLight.Position = Vector3(0, 1000, 0)
testLight.Direction = Vector3(0, -1, 0)
testLight.Diffuse = RGBA(1, 1, 1, 1)
testLight.Specular = RGBA(1, 1, 1, 1)
testLight.Ambient = RGBA(1, 1, 1, 1)
testLight.Type = Enum.LightType.Spot
testLight.InnerRadius = math.pi / 5
testLight.OuterRadius = math.pi / 4.1
testLight.Attenuation = Vector3(1, 0, 0.005)
testLight.Brightness = 1.5
testLight:SetShadowsEnabled(true, 1024, 1024)
testLight.Parent = lightOrb

scene:AddLight(testLight)
scene:AddObject(lightOrbModel)

local sceneDraw = GameObject("GlowingSceneOperation")
sceneDraw:SetParent(level)
sceneDraw:Configure(resolution.Width, resolution.Height, scene)
sceneDraw.Radius = 10
sceneDraw.Sigma = 20
sceneDraw.SkyBrightness = 1
sceneDraw.SkyBackgroundBrightness = 1
sceneDraw.SkyColor = RGBA(1, 1, 1, 1)
sceneDraw.SkyBackgroundColor = RGBA(0, 0, 0, 0)
sceneDraw.Resolution = Vector3(resolution.Width, resolution.Height)
sceneDraw.RenderAutomatically = true
sceneDraw.RangeFittingType = Enum.RangeFittingMode.Exposure
sceneDraw.Exposure = 0.025

local uiDraw = GameObject("InterfaceDrawOperation")
uiDraw.Parent = level
uiDraw.RenderAutomatically = true

local screen = GameObject("DeviceTransform")
screen.Size = DeviceVector(0, GameObject.FrameBuffer.WindowSize.Width, 0, GameObject.FrameBuffer.WindowSize.Height)
screen.Parent = uiDraw

uiDraw.CurrentScreen = screen

local frameTransform = GameObject("DeviceTransform")
frameTransform.Size = DeviceVector(0.25, 0, 0.25, 0)
frameTransform.Parent = screen

local frameStencil = GameObject("CanvasStencil")
frameStencil.Parent = frameTransform

local frameCanvas = GameObject("ScreenCanvas")
frameCanvas.Parent = frameTransform

local frameAppearance = GameObject("Appearance")
frameAppearance.Texture = sceneDraw:GetSceneBuffer():GetTexture(0)

frameCanvas.Appearance = frameAppearance

local modelTransform = GameObject("Transform")
modelTransform:SetParent(Engine.Environments.Level.Simulation)

local model = GameObject("Model")
model.MaterialProperties = material
model.Asset = meshes.Sphere
model:SetParent(modelTransform)

local boundingSphereTransform = GameObject("Transform")
boundingSphereTransform:SetParent(Engine.Environments.Level.Simulation)

local boundingSphereModel = GameObject("Model")
boundingSphereModel.Color = RGBA(1, 1, 1, 0.99)
boundingSphereModel.MaterialProperties = material
boundingSphereModel.Asset = meshes.WireSphere
boundingSphereModel:SetParent(boundingSphereTransform)

scene:AddObject(model)
scene:AddObject(boundingSphereModel)

local selectedModel = 0
local selectedBuffer = 1
local selectedMethod = 1

local methods = { "ComputeCentroid", "ComputeRitter", "ComputeLarson", "ComputePCA" }

coroutine.wrap(function()
	print(pcall(function()
		local userInput = Engine.GameWindow.UserInput

		local keyW = userInput:GetInput(Enum.InputCode.W)
		local keyA = userInput:GetInput(Enum.InputCode.A)
		local keyS = userInput:GetInput(Enum.InputCode.S)
		local keyD = userInput:GetInput(Enum.InputCode.D)
		local keyQ = userInput:GetInput(Enum.InputCode.Q)
		local keyE = userInput:GetInput(Enum.InputCode.E)
		local keyLeftArrow = userInput:GetInput(Enum.InputCode.LeftArrow)
		local keyRightArrow = userInput:GetInput(Enum.InputCode.RightArrow)
		local keyUpArrow = userInput:GetInput(Enum.InputCode.UpArrow)
		local keyDownArrow = userInput:GetInput(Enum.InputCode.DownArrow)
		local keyBracketOpen = userInput:GetInput(Enum.InputCode.BracketOpen)
		local keyBracketClose = userInput:GetInput(Enum.InputCode.BracketClose)
		local keySpace = userInput:GetInput(Enum.InputCode.Space)
		local keyAlt = userInput:GetInput(Enum.InputCode.LeftAlt)
		local keyTab = userInput:GetInput(Enum.InputCode.Tab)
		local mouseRight = userInput:GetInput(Enum.InputCode.MouseRight)
		local mousePosition = userInput:GetInput(Enum.InputCode.MousePosition)
		local mouseWheel = userInput:GetInput(Enum.InputCode.MouseWheel)

		local speed = 20
		local defaultSpeed = 20
		local fastSpeed = 100
		local aspectRatio = GameObject.FrameBuffer.WindowSize.Width / GameObject.FrameBuffer.WindowSize.Height
		local size = 5 / 3
		local defaultWidth = aspectRatio * size
		local defaultHeight = size
		local defaultProjection = 1
		local defaultNear = 0.1
		local defaultFar = 10000

		local pitch = 0
		local yaw = 0

		local previousPosition = mousePosition:GetPosition()
		
		local time = 0
		local modelChangeDebounce = true
		local bufferChangeDebounce = true
		local sphereChangeDebounce = true
		local boundingSphere = BoundingSphere[methods[selectedMethod]](meshes:Get(selectedModel))
		
		local outputBuffers = {
			sceneDraw:GetSceneBuffer():GetTexture(0),
			sceneDraw:GetSceneBuffer():GetTexture(1),
			sceneDraw:GetSceneBuffer():GetTexture(2),
			sceneDraw:GetSceneBuffer():GetTexture(3),
			sceneDraw:GetSceneBuffer():GetTexture(4),
			sceneDraw:GetSceneBuffer():GetTexture(5),
			sceneDraw:GetSceneBuffer():GetTexture(6),
			sceneDraw:GetLuminescenceBuffer():GetTexture(),
			sceneDraw:GetHorizontalPass():GetTexture(),
			sceneDraw:GetVerticalPass():GetTexture()
		}

		while true do
			local delta = wait(1 / 60)
			
			time = time + delta

			local x = 0
			local y = 0
			local z = 0

			if keyW:GetState() then
				z = -delta * speed
			end

			if keyS:GetState() then
				z = delta * speed
			end
			
			if keyA:GetState() then
				x = -delta * speed
			end

			if keyD:GetState() then
				x = delta * speed
			end

			if keyQ:GetState() then
				y = -delta * speed
			end
			
			if keyE:GetState() then
				y = delta * speed
			end
			
			if keySpace:GetState() then
				speed = fastSpeed
			else
				speed = defaultSpeed
			end
			
			if modelChangeDebounce and keyLeftArrow:GetStateChanged() and keyLeftArrow:GetState() then
				selectedModel = selectedModel - 1
				
				if selectedModel < 0 then
					selectedModel = meshes:GetChildren() - 1
				end
				
				boundingSphere = BoundingSphere[methods[selectedMethod]](meshes:Get(selectedModel))
			end
			
			if modelChangeDebounce and keyRightArrow:GetStateChanged() and keyRightArrow:GetState() then
				selectedModel = selectedModel + 1
				
				if selectedModel >= meshes:GetChildren() then
					selectedModel = 0
				end
				
				boundingSphere = BoundingSphere[methods[selectedMethod]](meshes:Get(selectedModel))
			end
			
			modelChangeDebounce = not (keyLeftArrow:GetState() or keyRightArrow:GetState())
			
			if bufferChangeDebounce and keyUpArrow:GetStateChanged() and keyUpArrow:GetState() then
				selectedBuffer = selectedBuffer - 1
				
				if selectedBuffer < 1 then
					selectedBuffer = #outputBuffers
				end
			end
			
			if bufferChangeDebounce and keyDownArrow:GetStateChanged() and keyDownArrow:GetState() then
				selectedBuffer = selectedBuffer + 1
				
				if selectedBuffer > #outputBuffers then
					selectedBuffer = 1
				end
			end
			
			bufferChangeDebounce = not (keyUpArrow:GetState() or keyDownArrow:GetState())
			
			if sphereChangeDebounce and keyBracketClose:GetStateChanged() and keyBracketClose:GetState() then
				selectedMethod = selectedMethod - 1
				
				if selectedMethod < 1 then
					selectedMethod = #methods
				end
				
				boundingSphere = BoundingSphere[methods[selectedMethod]](meshes:Get(selectedModel))
			end
			
			if sphereChangeDebounce and keyBracketOpen:GetStateChanged() and keyBracketOpen:GetState() then
				selectedMethod = selectedMethod + 1
				
				if selectedMethod > #methods then
					selectedMethod = 1
				end
				
				boundingSphere = BoundingSphere[methods[selectedMethod]](meshes:Get(selectedModel))
			end
			
			sphereChangeDebounce = not (keyBracketClose:GetState() or keyBracketOpen:GetState())

			if keyTab:GetState() then
				testLight.Position = camera:GetTransformation():Translation()
				testLight.Direction = -camera:GetTransformation():FrontVector()
				lightOrb:SetPosition(testLight.Position)
				lightOrb:Update(0)
			end
			
			model.Asset = meshes:Get(selectedModel)
			frameAppearance.Texture = outputBuffers[selectedBuffer]
			
			local size = model.Asset:GetSize()
			local scale = 1 / math.max(math.max(size.X, size.Y), size.Z)
			
			boundingSphereTransform.Transformation = Matrix3.NewScale(scale, scale, scale) * Matrix3(boundingSphere.Center - model.Asset:GetCenter()) * Matrix3.NewScale(boundingSphere.Radius, boundingSphere.Radius, boundingSphere.Radius)
			
			modelTransform.Transformation = Matrix3.NewScale(scale, scale, scale) * Matrix3(-model.Asset:GetCenter())

			if mouseRight:GetState() then
				local currentPosition = mousePosition:GetPosition()
				local mouseDelta = currentPosition - previousPosition

				yaw = (yaw - mouseDelta.X * delta * 0.1 + 2 * math.pi) % (2 * math.pi)
				pitch = math.min(math.max(pitch - mouseDelta.Y * delta * 0.1, -math.pi / 2), math.pi / 2)

				Engine.GameWindow:SetMousePosition(previousPosition)
			else
				previousPosition = mousePosition:GetPosition()
			end

			local transform = camera:GetTransformation() * Matrix3(x, y, z)
			local rotation = (Matrix3.YawRotation(yaw) * Matrix3.PitchRotation(pitch)):TransformedAround(transform:Translation())
			
			camera:SetTransformation(rotation * Matrix3(transform:Translation()))

			scene:Update(0)
			
			for i, light in pairs(lights) do
				light.Transformation = Matrix3.YawRotation(i * math.pi / 4 + time) * Matrix3(0, 0, 8) * Matrix3.NewScale(0.25, 0.25,0.25)
				light.Light.Position = light.Transformation:Translation()
			end
		end
	end))
end)()