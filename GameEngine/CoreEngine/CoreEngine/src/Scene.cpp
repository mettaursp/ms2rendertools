#include "Scene.h"

#include <queue>

#include "Materials.h"
#include "Graphics.h"
#include "LuaBinding.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"
#include "Terrain.h"
#include "LightBuffer.h"
#include "Transform.h"

namespace GraphicsEngine
{
	SceneObject::~SceneObject()
	{
		while (Scenes.size() != 0)
		{
			auto scene = Scenes.back().Scene.lock();

			if (scene != nullptr)
				scene->RemoveObjectRaw(this);
		}
	}

	void SceneObject::ParentChanged(std::shared_ptr<Object> newParent)
	{
		std::shared_ptr<Engine::Transform> transform;

		if (newParent->IsA<Engine::Transform>())
			transform = newParent->Cast<Engine::Transform>();
		else
			transform = GetComponent<Engine::Transform>();

		TransformChangedConnection = transform->TransformMoved.ConnectWithObject([this](Engine::Transform*) {
			UpdateScenes();

			return true;
		}, This.lock());

		UpdateScenes();
	}

	void SceneObject::UpdateScenes()
	{
		bool isStatic = IsStatic();

		for (int i = 0; i < int(Scenes.size()); ++i)
		{
			auto scene = Scenes[i].Scene.lock();

			if (scene != nullptr)
				scene->Update(Scenes[i].Reference, isStatic);
		}
	}

	void SceneObject::AddedToScene(std::shared_ptr<Scene>& scene, int reference)
	{
		Scenes.push_back({ reference, scene });

		UpdateScenes();
	}

	void SceneObject::RemovedFromScene(std::shared_ptr<Scene>& scene)
	{
		for (int i = 0; i < int(Scenes.size()); ++i)
		{
			if (Scenes[i].Scene.expired())
			{
				std::swap(Scenes[i], Scenes.back());
				Scenes.pop_back();

				--i;

				continue;
			}

			if (Scenes[i].Scene.lock() == scene)
			{
				std::swap(Scenes[i], Scenes.back());
				Scenes.pop_back();

				return;
			}
		}
	}
	void SceneObject::SetMaterial(const std::shared_ptr<Material>& material)
	{
		MaterialProperties = material;
		MaterialPropertiesObject = material.get();
	}

	std::shared_ptr<Material> SceneObject::GetMaterial() const
	{
		return MaterialProperties.lock();
	}

	Material* SceneObject::GetMaterialRaw() const
	{
		return MaterialPropertiesObject;
	}

	void Scene::RefreshWatches()
	{
		UpdatedLights.Clear();

		auto currentCamera = CurrentCamera.lock();

		for (int i = 0; i < GetLights(); ++i)
		{
			std::shared_ptr<Light> light = GetLight(i);

			if (light->Enabled)
			{
				light->RecomputeRadius();

				if (light->AreShadowsEnabled() && currentCamera->GetFrustum().Intersects(light->GetBoundingBox()) != Enum::IntersectionType::Outside)
				{
					Aabb boundingBox = light->GetBoundingBox();

					if (RefreshLightWatch(i))
						UpdatedLights.Insert((char*)nullptr + i, boundingBox);
					else
					{
						DynamicUpdatedLights.Clear();
						DynamicUpdatedLights.Insert(0, boundingBox);

						bool inserted = false;

						DynamicObjects.PairQuery(DynamicUpdatedLights, [this, &inserted] (const AabbTree::Node* objectNode, const AabbTree::Node* lightNode)
						{
							inserted = true;
						});

						if (inserted)
						{
							Lights[i].Watch.Clear();
							UpdatedLights.Insert((char*)nullptr + i, boundingBox);
						}
					}
				}
			}
		}

		StaticObjects.PairQuery(UpdatedLights, [this] (const AabbTree::Node* objectNode, const AabbTree::Node* lightNode)
		{
			SceneObjectReference* object = objectNode->GetData<SceneObjectReference>();
			int lightIndex = int(lightNode->ClientData);

			if (!object->Reference->IsTransparent())
				Lights[lightIndex].Watch.Insert(object, object->Reference->GetBoundingBox());
		});

		DynamicObjects.PairQuery(UpdatedLights, [this] (const AabbTree::Node* objectNode, const AabbTree::Node* lightNode)
		{
			SceneObjectReference* object = objectNode->GetData<SceneObjectReference>();
			int lightIndex = int(lightNode->ClientData);

			if (!object->Reference->IsTransparent())
				Lights[lightIndex].Watch.Insert(object, object->Reference->GetBoundingBox());
		});
	}

	bool Scene::RefreshLightWatch(int lightIndex)
	{
		Aabb box = Lights[lightIndex].Reference.lock()->GetBoundingBox();

		if (box.Min != Lights[lightIndex].LastBox.Min || box.Max != Lights[lightIndex].LastBox.Max)
		{
			Lights[lightIndex].LastBox = box;
			Lights[lightIndex].Watch.Clear();

			return true;
		}

		return false;
	}

	const AabbTree& Scene::GetWatched(int lightIndex) const
	{
		return Lights[lightIndex].Watch;
	}

	void Scene::CastRay(const Ray& ray, const CastResultsCallback& callback) const
	{
		std::priority_queue<SceneRayCastResults> hits;

		auto resultsProcessorLambda = [&hits, &ray, &callback] (const AabbTree::Node* node, float t)
		{
			SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

			objectReference->Reference->CastRay(ray, [&hits, &callback] (const SceneRayCastResults& results)
			{
				hits.push(results);
			});
		};

		StaticObjects.CastRay(ray, std::ref(resultsProcessorLambda));

		DynamicObjects.CastRay(ray, std::ref(resultsProcessorLambda));

		while (!hits.empty())
		{
			callback(hits.top());
		
			hits.pop();
		}
	}

	int Scene::CastRay(lua_State* lua)
	{
		int arguments;

		Engine::LuaTypes::Lua_Ray::Converter<Ray> rayValue(0, arguments, false);

		rayValue.FuncName = "CastRay";
		rayValue.LuaState = &lua;

		Ray ray = (Ray)rayValue;

		if (lua_type(lua, 3) != LUA_TFUNCTION)
			Lua::BadArgumentError(lua, 2, "function", Lua::GetType(lua, 3), "CastRay");

		int top = lua_gettop(lua);
		bool stopped = false;

		CastRay(ray, [&lua, &stopped](const SceneRayCastResults& results)
		{
			if (stopped) return;

			int top = lua_gettop(lua);

			lua_pushvalue(lua, 3);

			Engine::LuaTypes::Lua_SceneRayCastResults::ReturnValue returned;

			returned.LuaState = &lua;

			returned(results);

			lua_call(lua, returned.ReturnValues, 1);

			if (lua_isboolean(lua, 4) && lua_toboolean(lua, 4))
				stopped = true;

			lua_pop(lua, 1);
		});

		return 0;
	}

	SceneObject* Scene::GetStaticObject(const AabbTree::Node* node) const
	{
		SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

		return objectReference->Reference;
	}

	SceneObject* Scene::GetDynamicObject(const AabbTree::Node* node) const
	{
		SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

		return objectReference->Reference;
	}

	void Scene::ClearStaticObjects()
	{
		StaticObjects.Clear();

		for (int i = 0; i < int(StaticObjectReferences.size()); ++i)
			StaticObjectHandles.Destroy(StaticObjectReferences[i]);

		StaticObjectReferences.clear();
	}

	void Scene::Initialize()
	{
		//SetTicks(true);
	}

	void Scene::Update(float)
	{
	}

	void Scene::AddObject(const std::shared_ptr<SceneObject>& object)
	{
		int id = -1;

		if (object->IsStatic())
		{
			SceneObjectReference* handle = StaticObjectHandles.Create<SceneObjectReference>(This.lock(), object);

			id = StaticObjects.Insert(handle, object->GetBoundingBox());
			handle->ID = id;

			StaticObjectReferences.push_back(handle);
		}
		else
		{
			SceneObjectReference* handle = StaticObjectHandles.Create<SceneObjectReference>(This.lock(), object);

			id = DynamicObjects.Insert(handle, object->GetBoundingBox());
			handle->ID = id;

			DynamicObjectReferences.push_back(handle);
		}

		auto scene = This.lock()->Cast<Scene>();
		object->AddedToScene(scene, id);
	}

	void Scene::AddLight(const std::shared_ptr<Light>& light)
	{
		Lights.push_back(LightReference());

		Lights.back().Reference = light;
	}

	void Scene::AddTerrain(const std::shared_ptr<Engine::Terrain>& terrain)
	{
		Terrains.push_back(std::weak_ptr<Engine::Terrain>());

		Terrains.back() = terrain;
	}

	void Scene::RemoveObject(const std::shared_ptr<SceneObject>& object)
	{
		RemoveObjectRaw(object.get());
	}

	void Scene::RemoveObjectRaw(SceneObject* object)
	{
		if (object->IsStatic())
		{
			int i = 0;

			for (i = 0; i < int(StaticObjectReferences.size()) && StaticObjectReferences[i]->Reference != object; ++i);

			if (i < int(StaticObjectReferences.size()))
			{
				StaticObjects.Remove(StaticObjectReferences[i]->ID);

				StaticObjectHandles.Destroy(StaticObjectReferences[i]);

				std::swap(StaticObjectReferences[i], StaticObjectReferences.back());

				StaticObjectReferences.pop_back();
			}
		}
		else
		{
			int i = 0;

			for (i = 0; i < int(DynamicObjectReferences.size()) && DynamicObjectReferences[i]->Reference != object; ++i);

			if (i < int(DynamicObjectReferences.size()))
			{
				DynamicObjects.Remove(DynamicObjectReferences[i]->ID);

				StaticObjectHandles.Destroy(DynamicObjectReferences[i]);

				std::swap(DynamicObjectReferences[i], DynamicObjectReferences.back());

				DynamicObjectReferences.pop_back();
			}
		}

		auto scene = This.lock()->Cast<Scene>();
		object->RemovedFromScene(scene);
	}

	void Scene::RemoveLight(const std::shared_ptr<Light>& light)
	{
		LightVector::iterator i;

		for (i = Lights.begin(); i != Lights.end() && i->Reference.lock() != light; ++i);

		if (i != Lights.end())
			Lights.erase(i);
	}

	void Scene::RemoveTerrain(const std::shared_ptr<Engine::Terrain>& terrain)
	{
		TerrainVector::iterator i;

		for (i = Terrains.begin(); i != Terrains.end() && i->lock() != terrain; ++i);

		if (i != Terrains.end())
			Terrains.erase(i);
	}

	int Scene::GetLights() const
	{
		return int(Lights.size());
	}

	std::shared_ptr<Light> Scene::GetLight(int i) const
	{
		return Lights[i].Reference.lock();
	}

	int Scene::GetObjects() const
	{
		return int(Objects.size());
	}

	std::shared_ptr<SceneObject> Scene::GetObject(int i) const
	{
		return Objects[i].lock();
	}

	namespace
	{
		LightBuffer lightBuffer;
	}

	void Scene::Draw(bool drawTransparent, const std::shared_ptr<Camera>& targetCamera) const
	{
		const std::shared_ptr<Camera>& camera = targetCamera == nullptr ? CurrentCamera.lock() : targetCamera;

		for (int i = 0; i < int(Objects.size()); ++i)
		{
			std::shared_ptr<SceneObject> object = Objects[i].lock();

			if (object->Visible && object->IsTransparent() == drawTransparent && object->GetMaterialRaw() != nullptr)
			{
				if (drawTransparent)
					Programs::PhongForward->SetMaterial(object->GetMaterialRaw());
				else
					Programs::Phong->SetMaterial(object->GetMaterialRaw());

				object->Draw(camera);
			}
		}

		Draw(StaticObjects, drawTransparent, camera);
		Draw(DynamicObjects, drawTransparent, camera);
	}

	void Compare(SceneObject* left, SceneObject* right)
	{

	}

	void Scene::BuildRenderQueue(const std::shared_ptr<Camera>& targetCamera)
	{
		//if (targetCamera.get() == LastCamera && !targetCamera->Moved()) return;
		//
		//LastCamera = targetCamera.get();

		if (int(QueuedObjects.size()) < Material::GetMaterialCount())
			QueuedObjects.resize(Material::GetMaterialCount());

		for (int i = 0; i < Material::GetMaterialCount(); ++i)
		{
			QueuedObjects[i].TransparentObjectsStart = 0;
			QueuedObjects[i].QueuedObjects.clear();
		}

		//QueuedObjects.clear();
		//TransparentObjectsStart = 0;

		auto queueObject = [this](const AabbTree::Node* node)
		{
			SceneObjectReference* thingamajig = node->GetData<SceneObjectReference>();
			
			if (thingamajig->Reference->Visible)
			{
				Material* material = thingamajig->Reference->GetMaterialRaw();

				if (material == nullptr) return;

				MaterialQueue& queue = QueuedObjects[material->GetMaterialId()];

				queue.QueuedObjects.push_back(thingamajig->Reference);

				if (!thingamajig->Reference->IsTransparent())
				{
					if (queue.TransparentObjectsStart + 1 < queue.QueuedObjects.size())
						std::swap(queue.QueuedObjects[queue.TransparentObjectsStart], queue.QueuedObjects.back());

					++queue.TransparentObjectsStart;

					std::push_heap(queue.QueuedObjects.begin(), queue.QueuedObjects.begin() + queue.TransparentObjectsStart);
				}
			}
		};

		StaticObjects.CastFrustum(targetCamera->GetFrustum(), queueObject);
		DynamicObjects.CastFrustum(targetCamera->GetFrustum(), queueObject);
	}

	void Scene::DrawQueued(bool drawTransparent)
	{
		if (drawTransparent)
		{
			for (int j = 0; j < Material::GetMaterialCount(); ++j)
			{
				Material* material = Material::GetMaterialFromId(j);

				if (material == nullptr) continue;

				MaterialQueue& queue = QueuedObjects[j];

				Programs::PhongForward->SetMaterial(material);

				for (int i = queue.TransparentObjectsStart; i < int(queue.QueuedObjects.size()); ++i)
				{

					queue.QueuedObjects[i]->Draw(nullptr);
				}
			}
		}
		else
		{
			//for (int i = 0; i < TransparentObjectsStart; ++i)
			//{
			//	Programs::Phong->SetMaterial(QueuedObjects[i]->GetMaterialRaw());
			//
			//	QueuedObjects[i]->Draw(nullptr);
			//}
			for (int j = 0; j < Material::GetMaterialCount(); ++j)
			{
				Material* material = Material::GetMaterialFromId(j);

				if (material == nullptr) continue;

				MaterialQueue& queue = QueuedObjects[j];

				Programs::Phong->SetMaterial(material);

				for (int i = 0; i < int(queue.TransparentObjectsStart); ++i)
				{

					queue.QueuedObjects[i]->Draw(nullptr);
				}
			}
		}
		//int start = drawTransparent ? TransparentObjectsStart : 0;
		//int end = drawTransparent ? int(QueuedObjects.size()) : TransparentObjectsStart;
		//
		//for (int i = start; i < end; ++i)
		//{
		//	if (drawTransparent)
		//		Programs::PhongForward->SetMaterial(QueuedObjects[i]->GetMaterialRaw());
		//	else
		//		Programs::Phong->SetMaterial(QueuedObjects[i]->GetMaterialRaw());
		//
		//	QueuedObjects[i]->Draw(nullptr);
		//}
	}

	void Scene::StackInfo::operator()(const AabbTree::Node* node) const
	{
		SceneObjectReference* thingamajig = node->GetData<SceneObjectReference>();

		if (thingamajig->Reference->Visible & (thingamajig->Reference->IsTransparent() == DrawTransparent) & (thingamajig->Reference->GetMaterialRaw() != nullptr))
		{
			//++Drawn;

			if (DrawTransparent && ShaderProgram::GetCurrentProgram() == Programs::PhongForward)
				Programs::PhongForward->SetMaterial(thingamajig->Reference->GetMaterialRaw());
			else if (ShaderProgram::GetCurrentProgram() == Programs::Phong)
				Programs::Phong->SetMaterial(thingamajig->Reference->GetMaterialRaw());

			thingamajig->Reference->Draw(TargetCamera);
		}
	}

	void Scene::Draw(const AabbTree& tree, bool drawTransparent, const std::shared_ptr<Camera>& targetCamera)
	{
		int drawn = 0;
		StackInfo FrustumCallbackInfo{ drawTransparent, targetCamera, drawn };

		tree.CastFrustumFast(targetCamera->GetFrustum(), FrustumCallbackInfo);

		drawn += 0;
	}

	void Scene::DrawTerrain(bool drawLiquid, const std::shared_ptr<Camera>& targetCamera)
	{
		const std::shared_ptr<Camera>& camera = targetCamera == nullptr ? CurrentCamera.lock() : targetCamera;

		for (int i = 0; i < int(Terrains.size()); ++i)
			Terrains[i].lock()->Draw(drawLiquid, camera);
	}

	void Scene::Update(int object)
	{
		if (!Updating)
			throw "Attempt to perform unauthorized swap";

		UpdateVector.push_back(object);
	}

	void Scene::Update(int object, bool isStatic)
	{
		AabbTree& tree = isStatic ? StaticObjects : DynamicObjects;
		const AabbTree& constTree = tree;

		tree.Update(object, constTree.GetNode(object)->GetData<SceneObjectReference>()->Reference->GetBoundingBox());
	}

	std::shared_ptr<Camera>& Scene::Target()
	{
		static std::shared_ptr<Camera> camera = nullptr;

		return camera;
	}
}
