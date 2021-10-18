#pragma once

#include "Object.h"
#include "AabbTree.h"
#include "PageAllocator.h"
#include "Ray.h"
#include "SceneRayCastResults.h"
#include "RGBA.h"

namespace Engine
{
	class Terrain;
}

namespace GraphicsEngine
{
	class Camera;
	class Light;
	class Material;
	class PhysicalMaterial;

	class SceneObject : public Engine::Object
	{
	public:
		typedef std::function<void(const SceneRayCastResults& results)> CastResultsCallback;

		virtual ~SceneObject() {}

		void Initialize() {}
		void Update(float) {}

		bool Visible = true;
		std::weak_ptr<Material> MaterialProperties;
		std::weak_ptr<PhysicalMaterial> PhysicalMaterialProperties;

		virtual Aabb GetBoundingBox() const { return Aabb(); }
		virtual bool HasMoved() const { return false; }
		virtual bool IsStatic() const { return false; }
		virtual void Draw(const std::shared_ptr<Camera>& camera) {}
		virtual bool IsTransparent() const { return false; }
		virtual void CastRay(const Ray& ray, const CastResultsCallback& callback) const {}

		Instantiable;

		Inherits_Class(Object);

		Reflected(SceneObject);
	};
}

namespace Engine
{
	//Class_Inherits(GraphicsEngine::SceneObject, Object);
}

struct lua_State;

namespace GraphicsEngine
{
	class Scene : public Engine::Object
	{
	public:
		typedef std::vector<int> IDVector;
		typedef std::function<void(const SceneRayCastResults& results)> CastResultsCallback;

		virtual ~Scene() {}

		void Initialize() {}
		void Update(float);

		std::weak_ptr<Camera> CurrentCamera;
		std::weak_ptr<Light> GlobalLight;

		RGBA FogColor = 0xFFFFFFFF;
		float FogNear = 3;
		float FogFar = 25;

		RGBA GlobalDiffuse = 0xFFFFFFFF;
		RGBA GlobalSpecular = 0xFFFFFFFF;
		RGBA GlobalAmbient = 0xFFFFFFFF;

		void RefreshWatches();
		const AabbTree& GetWatched(int lightIndex) const;

		void CastRay(const Ray& ray, const CastResultsCallback& callback) const;
		int CastRay(lua_State* lua);

		int GetObjectID(const AabbTree::Node* node) const { return node->GetData<SceneObjectReference>()->Reference.lock()->GetObjectID(); }
		const AabbTree& GetStaticObjects() const { return StaticObjects; }
		const AabbTree& GetDynamicObjects() const { return DynamicObjects; }
		int GetStaticObjectCount() const { return int(StaticObjectReferences.size()); }
		int GetDynamicbjectCount() const { return int(DynamicObjectReferences.size()); }
		std::shared_ptr<SceneObject> GetStaticObject(const AabbTree::Node* node) const;
		std::shared_ptr<SceneObject> GetDynamicObject(const AabbTree::Node* node) const;
		void ClearStaticObjects();
		void AddObject(const std::shared_ptr<SceneObject>& object);
		void AddLight(const std::shared_ptr<Light>& light);
		void AddTerrain(const std::shared_ptr<Engine::Terrain>& terrain);
		void RemoveObject(const std::shared_ptr<SceneObject>& object);
		void RemoveLight(const std::shared_ptr<Light>& light);
		void RemoveTerrain(const std::shared_ptr<Engine::Terrain>& terrain);
		int GetLights() const;
		std::shared_ptr<Light> GetLight(int i) const;
		int GetObjects() const;
		std::shared_ptr<SceneObject> GetObject(int i) const;

		void DrawTerrain(bool drawLiquid, const std::shared_ptr<Camera>& targetCamera = nullptr);
		void Draw(bool drawTransparent = false, const std::shared_ptr<Camera>& targetCamera = nullptr) const;
		void Update(int object);

		static void Draw(const AabbTree& tree, bool drawTransparent = false, const std::shared_ptr<Camera>& targetCamera = Target());

	private:
		struct SceneObjectReference
		{
			std::weak_ptr<SceneObject> Reference;
			int ID = -1;

			SceneObjectReference(const std::shared_ptr<Object>& parent, const std::shared_ptr<SceneObject>& object)
			{
				Reference = object;
			}
		};

		struct LightReference
		{
			AabbTree Watch;
			std::weak_ptr<Light> Reference;
			Aabb LastBox;
		};

		typedef std::vector<LightReference> LightVector;
		typedef std::vector<int> KeyVector;

		typedef ClassAllocator<SceneObjectReference> HandleAllocator;
		typedef std::vector<SceneObjectReference*> ObjectReferenceVector;
		typedef std::vector<std::weak_ptr<SceneObject>> ObjectVector;
		typedef std::vector<std::weak_ptr<Engine::Terrain>> TerrainVector;

		AabbTree UpdatedLights;
		AabbTree DynamicUpdatedLights;
		AabbTree StaticObjects;
		AabbTree DynamicObjects;
		HandleAllocator StaticObjectHandles;

		ObjectReferenceVector StaticObjectReferences;
		ObjectReferenceVector DynamicObjectReferences;
		ObjectVector Objects;
		LightVector Lights;
		TerrainVector Terrains;

		bool Updating = false;
		KeyVector UpdateVector;

		bool RefreshLightWatch(int lightIndex);

		static std::shared_ptr<Camera>& Target();

		Instantiable;

		Inherits_Class(Object);

		Reflected(Scene);
	};
}

namespace Engine
{
	//Class_Inherits(GraphicsEngine::Scene, Object);
}
