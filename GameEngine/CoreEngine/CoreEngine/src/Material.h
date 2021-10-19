#pragma once

#include "RGBA.h"
#include "Object.h"

namespace GraphicsEngine
{
	class Texture;

	class Material : public Engine::Object
	{
	public:
		RGBA Diffuse = RGBA(0.5f, 0.5f, 0.5f, 1);
		RGBA Specular = RGBA(0.5f, 0.5f, 0.5f, 1);
		RGBA Ambient = RGBA(0.1f, 0.1f, 0.1f, 1);
		RGBA Emission = RGBA(0, 0, 0, 0);
		int Shininess = 1;

		RGBA TextureColor;
		bool BlendTexture = true;
		Vector3 UVScale = Vector3(1, 1);
		Vector3 UVOffset;
		Vector3 BoxScale = Vector3(1, 1, 1);
		bool CubeMapped = false;
		bool FlipCubeMapV = false;
		bool CompressedNormalMap = false;
		bool UseOffsetMap = false;

		std::weak_ptr<Texture> NormalMap;
		std::weak_ptr<Texture> SpecularMap;
		std::weak_ptr<Texture> GlowMap;
		std::weak_ptr<Texture> CubeMapTop;
		std::weak_ptr<Texture> CubeMapBottom;
		std::weak_ptr<Texture> CubeMapLeft;
		std::weak_ptr<Texture> CubeMapRight;
		std::weak_ptr<Texture> CubeMapFront;
		std::weak_ptr<Texture> CubeMapBack;
		std::weak_ptr<Texture> DiffuseTexture;

		void Initialize() {}

		Instantiable;

		Inherits_Class(Object);

		Reflected(Material);
	};
}
