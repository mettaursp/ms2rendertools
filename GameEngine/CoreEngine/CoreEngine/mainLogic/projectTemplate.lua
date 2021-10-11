--[[
	Project template to set up a new 3D renderer scene
	
	Root object is in the variable 'GameObject'. Anything put inside this hierarchy is treated as a live game object.

	The engine is primarily a "subscription based" model where functionality is optional and needs to be subscribed to by
	making an object that handles it. An example would be a 3D render pipeline with a "GlowingSceneOperation" object.
]]

-- Create a home for mesh assets & load meshes in from a json file
local meshes = GameObject("Object")
meshes.Name = "Meshes"
meshes.Parent = Engine

local assets
print("pcall", pcall(function() assets = json.decode("./assets/json/mapAssets.json", true) end))

for name, path in pairs(assets.meshes) do
	print(name, path)
	GameObject.MeshLoader.NewAsset(name, path).Parent = meshes
	print("loaded", name)
end

-- Create a home for textures
local textures = GameObject("Textures")
textures.Name = "Textures"
textures.Parent = Engine


--[[
	Set up the effective game world. At the moment these don't do anything yet, but they are planned for use in controlling
	the simulation state. All live objects that are meant to be rendered or simulate something should be stored here.
]]
local environments = GameObject("Environments")
environments.Parent = Engine

local level = GameObject("Environment")
level.Name = "Level"
level.Parent = environments

local simulation = GameObject("Simulation")
simulation.Parent = level

--[[
	Initialize the global light that will be used to light up the scene.
	This is entirely optional, but it is good to set one up if you don't make any other lights and want to be able to see
	objects as anything other than black silhouettes.
]]

local skyLight = GameObject("Light")
skyLight.Enabled = true
skyLight.Direction = Vector3(0.25, -1, 0.25):Unit()
skyLight.Brightness = 0.5
skyLight.Diffuse = RGBA(1, 1, 1, 1)
skyLight.Specular = RGBA(1, 1, 1, 1)
skyLight.Ambient = RGBA(1, 1, 1, 1)
skyLight.Type = Enum.LightType.Directional
skyLight.Parent = simulation

-- Initialize the camera that will be used to render the scene. This can be moved around by script however you want.
local resolution = GameObject.FrameBuffer.WindowSize

local aspectRatio = resolution.Width / resolution.Height
local size = 5 / 3
local defaultWidth = aspectRatio * size
local defaultHeight = size
local defaultProjection = 1
local defaultNear = 0.1
local defaultFar = 5000

local camera = GameObject("Camera")
camera.Parent = level --[[ camera:SetParent(level) ]]
camera:SetProperties(defaultWidth, defaultHeight, defaultProjection, defaultNear, defaultFar)
camera:SetTransformation(Matrix3(0, 5, 10))

--[[
	Initialize the scene where objects are tracked for rendering purposes.
	It's primary purpose is to serve as an acceleration structure for render related lookups & to hold live render objects.
	If you want an object to render it has to be registered with `scene:AddObject(Model)`.
	If you want a light to illuminate the scene it has to be registered with `scene:AddLight(Light)`, except for the global light.
]]
local scene = GameObject("Scene")
scene.Parent = level
scene.CurrentCamera = camera
scene.GlobalLight = skyLight

-- Initialize the render pipeline
--[[
	This object is responsible for rendering objects to either the screen or a desired output FrameBuffer.
	A FrameBuffer can be used for render to texture, to render onto another object, or even saving to file (don't remember the state of this one lol)
]]
local sceneDraw = GameObject("GlowingSceneOperation")
sceneDraw.Parent = level
sceneDraw:Configure(resolution.Width, resolution.Height, scene)
sceneDraw.Radius = 10
sceneDraw.Sigma = 20
sceneDraw.SkyBrightness = 1
sceneDraw.SkyBackgroundBrightness = 1
sceneDraw.SkyColor = RGBA(15/255, 5/255, 15/255, 1)
sceneDraw.SkyBackgroundColor = RGBA(1, 0, 0, 0)
sceneDraw.Resolution = Vector3(resolution.Width, resolution.Height)
sceneDraw.RenderAutomatically = true


-- Add a script for controlling the camera in free cam mode
local freeCamSource = GameObject("LuaSource")
freeCamSource.Name = "FreeCamSource"
freeCamSource:LoadSource("./assets/scripts/freeCameraController.lua")

local freeCamScript = GameObject("LuaScript")
freeCamScript.Name = "FreeCamScript"
freeCamScript:SetSource(freeCamSource)
freeCamScript.Parent = camera

freeCamSource.Parent = freeCamScript

freeCamScript:Run()

-- Create some example objects in the scene
local defaultMaterial = GameObject("Material")
defaultMaterial.Shininess = 75
defaultMaterial.Diffuse = RGBA(0.5, 0.5, 0.5, 0)
defaultMaterial.Specular = RGBA(0.5, 0.5, 0.5, 0)
defaultMaterial.Ambient = RGBA(0.5, 0.5, 0.5, 0)
defaultMaterial.Emission = RGBA(0, 0, 0, 0)
defaultMaterial.Parent = level

function CreateObject(scene, meshAsset, material)
	if not meshAsset then
		print("CreateObject called with no mesh asset, defaulting to Core.Square")
	end
	if not material then
		print("CreateObject called with no material, defaulting to gray diffuse")
	end
	
	-- Many objects are built using transformation hierarchies. They search through their parents to find a Transform to inherit from.
	local transform = GameObject("Transform")
	
	--[[
		Models can be created that reference a material and mesh asset.
		The mesh asset is separate so it will only be created once and can be used across multiple objects.
		All 3D object rendering is tied to these models.
	]]
	local model = GameObject("Model")
	model.Parent = transform
	model.Asset = meshAsset or Engine.CoreMeshes.CoreCube
	model.MaterialProperties = material or defaultMaterial
	
	-- Register the model to the scene for rendering
	scene:AddObject(model)

	return transform
end

local objectsCreated = 0

for name in pairs(assets.meshes) do
	local object = CreateObject(scene, meshes[name])
	object.Transformation = Matrix3.EulerAnglesYawRotation(math.pi * objectsCreated / 5, 0, 0) * Matrix3(0, 0, 10)
	object.Parent = simulation
	object.Name = name
	
	print("Created", name)

	objectsCreated = objectsCreated + 1
end