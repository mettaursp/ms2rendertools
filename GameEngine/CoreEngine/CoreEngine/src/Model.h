#pragma once

#include <map>
#include <set>

#include "Mesh.h"
#include "RGBA.h"
#include "ShaderPrograms.h"
#include "Scene.h"

namespace Engine
{
	class Transform;
}

namespace GraphicsEngine
{
	class Texture;
	class ModelAsset;
	class Camera;

	class Model : public SceneObject
	{
	public:
		virtual ~Model() {}

		void Initialize();
		void Update(float);

		float Reflectivity = 0;
		//bool BlendTexture = true;
		RGBA Color;
		//RGBA TextureColor;
		RGBA GlowColor = RGBA(0, 0, 0, 1);
		//Vector3 UVScale = Vector3(1, 1);
		//Vector3 UVOffset;
		//Vector3 BoxScale = Vector3(1, 1, 1);
		//bool CubeMapped = false;
		//bool FlipCubeMapV = false;
		//bool CompressedNormalMap = false;
		//bool UseOffsetMap = false;
		//std::weak_ptr<Texture> NormalMap;
		//std::weak_ptr<Texture> SpecularMap;
		//std::weak_ptr<Texture> GlowMap;
		//std::weak_ptr<Texture> CubeMapTop;
		//std::weak_ptr<Texture> CubeMapBottom;
		//std::weak_ptr<Texture> CubeMapLeft;
		//std::weak_ptr<Texture> CubeMapRight;
		//std::weak_ptr<Texture> CubeMapFront;
		//std::weak_ptr<Texture> CubeMapBack;
		//std::weak_ptr<Texture> DiffuseTexture;
		std::weak_ptr<Engine::ModelAsset> Asset;

		void Draw(const std::shared_ptr<Camera>& camera);
		std::string GetMeshName() const;
		bool IsTransparent() const;
		Aabb GetBoundingBox() const;
		Aabb GetLocalBoundingBox() const;
		Matrix3 GetTransformation() const;
		Matrix3 GetInverseTransformation() const;
		bool HasMoved() const;
		bool IsStatic() const;
		void CastRay(const Ray& ray, const CastResultsCallback& callback) const;
		int GetMeshId() const;

	private:
		void DrawMesh(const std::shared_ptr<Camera>& camera, const Mesh* mesh) const;
		void DrawForward(const std::shared_ptr<Camera>& camera, const Mesh* mesh) const;
		void DrawShadow(const std::shared_ptr<Camera>& camera, const Mesh* mesh) const;
		void DrawDepth(const std::shared_ptr<Camera>& camera, const Mesh* mesh) const;

		Engine::Transform* TransformObject = nullptr;
		Engine::ModelAsset* AssetObject = nullptr;

		Instantiable;

		Inherits_Class(SceneObject);

		Reflected(Model);
	};
}