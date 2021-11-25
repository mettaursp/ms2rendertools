#include "Graphics.h"

#include <sstream>
extern "C" {
#include <Windows.h>
}

#include "Textures.h"
#include "Fonts.h"
#include "Materials.h"
#include "Engine.h"
#include "GameEngine.h"
#include "CoreMeshes.h"
#include "ModelAsset.h"
#include "EngineException.h"

Window* Graphics::ActiveWindow = nullptr;
Graphics* Graphics::System = nullptr;

PhongProgram* Programs::Phong = nullptr;
PhongOutputProgram* Programs::PhongOutput = nullptr;
PhongOutputStencilProgram* Programs::PhongOutputStencil = nullptr;
PhongForwardProgram* Programs::PhongForward = nullptr;
WaterMatrixProgram* Programs::WaterMatrix = nullptr;
WaterSimProgram* Programs::WaterSim = nullptr;
DepthUpdateProgram* Programs::DepthUpdate = nullptr;
DepthTraceProgram* Programs::DepthTrace = nullptr;
SkyboxProgram* Programs::SkyBox = nullptr;
ShadowMapProgram* Programs::ShadowMap = nullptr;
LuminescenceProgram* Programs::Luminescence = nullptr;
ToneMapProgram* Programs::ToneMap = nullptr;
PerlinGradientProgram* Programs::PerlinGradient = nullptr;
NoiseProgram* Programs::Noise = nullptr;
AutomataProgram* Programs::Automata = nullptr;
NebulaProgram* Programs::Nebula = nullptr;
BlurProgram* Programs::Blur = nullptr;
ScreenProgram* Programs::Screen = nullptr;
NormalMapGenProgram* Programs::NormalMapGen = nullptr;
BlendProgram* Programs::Blend = nullptr;
BlurBlendProgam* Programs::BlurBlend = nullptr;
ClippingMaskProgram* Programs::ClippingMask = nullptr;

namespace
{
	std::shared_ptr<Engine::ModelAsset> GetCore(const std::string& name)
	{
		return Engine::Root()->GetByName("CoreMeshes")->Cast<Engine::Object>()->GetByName(name)->Cast<Engine::ModelAsset>();
	}

	int GetCoreID(const std::string& name)
	{
		return GetCore(name)->GetMeshID();
	}

	template<typename T>
	T* init(T& program)
	{
		program.CoreMeshes = ShaderProgram::Primitives(
			program.Meshes->GetMesh(GetCoreID("CoreSquare")),
			program.Meshes->GetMesh(GetCoreID("CoreCube")),
			program.Meshes->GetMesh(GetCoreID("CoreWireCube")),
			program.Meshes->GetMesh(GetCoreID("CoreBoundingVolume")),
			program.Meshes->GetMesh(GetCoreID("CoreHalfBoundingVolume")),
			program.Meshes->GetMesh(GetCoreID("CoreCone")),
			program.Meshes->GetMesh(GetCoreID("CoreArrow")),
			program.Meshes->GetMesh(GetCoreID("CoreSphere")),
			program.Meshes->GetMesh(GetCoreID("CoreRing"))
		);

		return &program;
	}
}

void Graphics::Initialize()
{
	if (System != nullptr)
		throw EngineException("Graphics system already initialized");

	Graphics::ActiveWindow = new Window();
	System = new Graphics();

	Programs::Phong = init(System->Programs.Phong);
	Programs::PhongOutput = init(System->Programs.PhongOutput);
	Programs::PhongOutputStencil = init(System->Programs.PhongOutputStencil);
	Programs::PhongForward = init(System->Programs.PhongForward);
	Programs::WaterMatrix = init(System->Programs.WaterMatrix);
	Programs::WaterSim = init(System->Programs.WaterSim);
	Programs::DepthUpdate = init(System->Programs.DepthUpdate);
	Programs::DepthTrace = init(System->Programs.DepthTrace);
	Programs::ShadowMap = init(System->Programs.ShadowMap);
	Programs::SkyBox = init(System->Programs.SkyBox);
	Programs::Luminescence = init(System->Programs.Luminescence);
	Programs::ToneMap = init(System->Programs.ToneMap);
	Programs::PerlinGradient = init(System->Programs.PerlinGradient);
	Programs::Noise = init(System->Programs.Noise);
	Programs::Automata = init(System->Programs.Automata);
	Programs::Nebula = init(System->Programs.Nebula);
	Programs::Blur = init(System->Programs.Blur);
	Programs::Screen = init(System->Programs.Screen);
	Programs::NormalMapGen =  init(System->Programs.NormalMapGen);
	Programs::Blend = init(System->Programs.Blend);
	Programs::BlurBlend = init(System->Programs.BlurBlend);
	Programs::ClippingMask = init(System->Programs.ClippingMask);
}

void Graphics::Clean()
{
	if (System == nullptr)
		return;

	delete System;
	System = nullptr;
}

void Graphics::SetClearColor(const RGBA& color)
{
	glClearColor(color.R, color.G, color.B, color.A); CheckGLErrors();
}

void Graphics::ClearScreen(GLbitfield mask)
{
	glClear(mask); CheckGLErrors();
}

void Graphics::SetBlendMode(GLenum sMode, GLenum dMode)
{
	glBlendFunc(sMode, dMode); CheckGLErrors();
}

const std::shared_ptr<Engine::ModelAsset>& Graphics::GetCoreMesh(const std::string& name)
{
	static const std::shared_ptr<Engine::ModelAsset> null = nullptr;

	const auto& asset = System->CoreMeshAssets.find(name);

	if (asset != System->CoreMeshAssets.end())
		return asset->second;

	return null;
}

std::shared_ptr<Engine::ModelAsset> Graphics::LoadCore(const std::string& name, const MeshData& data, const std::shared_ptr<Engine::Object>& parent)
{
	std::shared_ptr<Engine::ModelAsset> model = MeshLoader::NewAsset(name, data);

	model->SetParent(parent);

	MeshLoader::SetCore(model->GetMeshID());

	CoreMeshAssets[name] = model;

	return model;
}

void Graphics::LoadAssets()
{
	{
		SquareData = CoreMeshes::Square();
		CubeData = CoreMeshes::Cube();
		WireCubeData = CoreMeshes::WireCube();
		BoundingVolumeData = CoreMeshes::BoundingVolume();
		HalfBoundingVolumeData = CoreMeshes::HalfBoundingVolume();
		ConeData = CoreMeshes::Cone();
		ArrowData = CoreMeshes::Arrow();
		SphereData = CoreMeshes::Sphere();
		RingData = CoreMeshes::Ring();

		std::shared_ptr<Engine::Object> coreMeshes = Engine::Create<Engine::Object>();

		coreMeshes->Name = "CoreMeshes";
		coreMeshes->SetParent(Engine::Root());

		CoreSquare = LoadCore("CoreSquare", SquareData, coreMeshes);
		CoreCube = LoadCore("CoreCube", CubeData, coreMeshes);
		CoreWireCube = LoadCore("CoreWireCube", WireCubeData, coreMeshes);
		CoreBoundingVolume = LoadCore("CoreBoundingVolume", BoundingVolumeData, coreMeshes);
		CoreHalfBoundingVolume = LoadCore("CoreHalfBoundingVolume", HalfBoundingVolumeData, coreMeshes);
		CoreCone = LoadCore("CoreCone", ConeData, coreMeshes);
		CoreArrow = LoadCore("CoreArrow", ArrowData, coreMeshes);
		CoreSphere = LoadCore("CoreSphere", SphereData, coreMeshes);
		CoreRing = LoadCore("CoreRing", RingData, coreMeshes);
	}
}

Graphics::Graphics()
{
	LoadAssets();
}

const char* getErrorName(GLenum error)
{
	switch (error)
	{
	case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
	case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
	case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
	case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_CONTEXT_LOST: return "GL_CONTEXT_LOST";
	case GL_TABLE_TOO_LARGE: return "GL_TABLE_TOO_LARGE";
	}

	return "";
}

void Graphics::CheckErrors(const char* file, int line, const char* func)
{
	GLenum error;

	bool errorsFound = false;

	do
	{
		error = glGetError();

		if (error != GL_NO_ERROR)
		{
			if (!errorsFound)
			{
				std::stringstream out;

				out << "OpenGL error: " << file << " (" << line << ") <" << func << ">: [" << error << "] " << getErrorName(error);

				std::cout << out.str() << std::endl;

				out << "\n";

				OutputDebugStringA(out.str().c_str());
			}

			errorsFound = true;
		}

	} while (error != GL_NO_ERROR);

	if (errorsFound)
		throw EngineException("OpenGL Errors found");
}
