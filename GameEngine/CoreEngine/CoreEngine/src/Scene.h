#pragma once

#include <queue>

#include "Object.h"
#include "AabbTree.h"
#include "PageAllocator.h"
#include "Ray.h"
#include "SceneRayCastResults.h"
#include "RGBA.h"
#include "Reference.h"

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
	class Scene;

	class SceneObject : public Engine::Object
	{
	public:
		typedef std::function<void(const SceneRayCastResults& results)> CastResultsCallback;

		virtual ~SceneObject();

		void Initialize() {}
		void Update(float) {}

		void ParentChanged(std::shared_ptr<Object> newParent);

		bool Visible = true;
		std::weak_ptr<PhysicalMaterial> PhysicalMaterialProperties;

		virtual Aabb GetBoundingBox() const { return Aabb(); }
		virtual Aabb GetLocalBoundingBox() const { return Aabb(); }
		virtual Matrix3 GetTransformation() const { return Matrix3(); }
		virtual Matrix3 GetInverseTransformation() const { return Matrix3(); }
		virtual bool HasMoved() const { return false; }
		virtual bool IsStatic() const { return false; }
		virtual void Draw(const std::shared_ptr<Camera>& camera) {}
		virtual bool IsTransparent() const { return false; }
		virtual bool UseDepthBuffer() const { return true; }
		virtual void CastRay(const Ray& ray, const CastResultsCallback& callback) const {}
		virtual int GetMeshId() const { return -1; }

		void AddedToScene(std::shared_ptr<Scene>& scene, int reference);
		void RemovedFromScene(std::shared_ptr<Scene>& scene);

		void SetMaterial(const std::shared_ptr<Material>& material);
		std::shared_ptr<Material> GetMaterial() const;
		Material* GetMaterialRaw() const;

	private:
		struct SceneReference
		{
			int Reference = -1;
			std::weak_ptr<Scene> Scene;
		};

		typedef std::vector<SceneReference> SceneVector;

		int TransformChangedConnection = -1;
		SceneVector Scenes;
		std::weak_ptr<Material> MaterialProperties;
		Reference<Material> MaterialPropertiesObject;

		void UpdateScenes();

		Instantiable;

		Inherits_Class(Object);

		Reflected(SceneObject);
	};
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

		void Initialize();
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

		int GetObjectID(const AabbTree::Node* node) const { return node->GetData<SceneObjectReference>()->Reference->GetObjectID(); }
		const AabbTree& GetStaticObjects() const { return StaticObjects; }
		const AabbTree& GetDynamicObjects() const { return DynamicObjects; }
		int GetStaticObjectCount() const { return int(StaticObjectReferences.size()); }
		int GetDynamicbjectCount() const { return int(DynamicObjectReferences.size()); }
		SceneObject* GetStaticObject(const AabbTree::Node* node) const;
		SceneObject* GetDynamicObject(const AabbTree::Node* node) const;
		void ClearStaticObjects();
		void AddObject(const std::shared_ptr<SceneObject>& object);
		void AddLight(const std::shared_ptr<Light>& light);
		void AddTerrain(const std::shared_ptr<Engine::Terrain>& terrain);
		void RemoveObject(const std::shared_ptr<SceneObject>& object);
		void RemoveObjectRaw(SceneObject* object);
		void RemoveLight(const std::shared_ptr<Light>& light);
		void RemoveTerrain(const std::shared_ptr<Engine::Terrain>& terrain);
		int GetLights() const;
		std::shared_ptr<Light> GetLight(int i) const;
		int GetObjects() const;
		std::shared_ptr<SceneObject> GetObject(int i) const;

		void BuildRenderQueue(const std::shared_ptr<Camera>& targetCamera);
		void DrawQueued(bool drawTransparent = false);
		void DrawTerrain(bool drawLiquid, const std::shared_ptr<Camera>& targetCamera = nullptr);
		void Draw(bool drawTransparent = false, const std::shared_ptr<Camera>& targetCamera = nullptr) const;
		void Update(int object);
		void Update(int object, bool isStatic);

		static void Draw(const AabbTree& tree, bool drawTransparent = false, const std::shared_ptr<Camera>& targetCamera = Target());

	private:
		struct SceneObjectReference
		{
			SceneObject* Reference;
			int ID = -1;

			SceneObjectReference(const std::shared_ptr<Object>& parent, const std::shared_ptr<SceneObject>& object)
			{
				Reference = object.get();
			}
		};

		struct LightReference
		{
			AabbTree Watch;
			std::weak_ptr<Light> Reference;
			Aabb LastBox;
		};

		struct StackInfo : public AabbTree::FastCallback
		{
			bool DrawTransparent;
			const std::shared_ptr<Camera>& TargetCamera;
			int& Drawn;

			StackInfo(bool drawTransparent, const std::shared_ptr<Camera>& targetCamera, int& drawn) : DrawTransparent(drawTransparent), TargetCamera(targetCamera), Drawn(drawn) {}

			void operator()(const AabbTree::Node* node) const;
		};

		typedef std::vector<LightReference> LightVector;
		typedef std::vector<int> KeyVector;

		typedef ClassAllocator<SceneObjectReference> HandleAllocator;
		typedef std::vector<SceneObjectReference*> ObjectReferenceVector;
		typedef std::vector<std::weak_ptr<SceneObject>> ObjectVector;
		typedef std::vector<std::weak_ptr<Engine::Terrain>> TerrainVector;

		struct RenderQueueItem
		{
			bool IsTransparent = false;
			int MeshId = -1;
			Material* MaterialProperties = nullptr;
			SceneObject* Item = nullptr;

			bool operator<(const RenderQueueItem& other) const
			{
				if (IsTransparent != other.IsTransparent)
					return IsTransparent;

				if (MaterialProperties != other.MaterialProperties)
					return MaterialProperties < other.MaterialProperties;

				return MeshId < other.MeshId;
			}
		};

		struct MaterialQueue
		{
			size_t TransparentObjectsStart = -1;
			std::vector<SceneObject*> QueuedObjects;
		};

		typedef std::priority_queue<RenderQueueItem> RenderQueue;

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
		RenderQueue ObjectRenderQueue;
		std::vector<MaterialQueue> QueuedObjects;

		size_t TransparentObjectsStart = -1;

		Camera* LastCamera = nullptr;

		bool Updating = false;
		KeyVector UpdateVector;

		bool RefreshLightWatch(int lightIndex);

		static std::shared_ptr<Camera>& Target();

		Instantiable;

		Inherits_Class(Object);

		Reflected(Scene);
	};
}
