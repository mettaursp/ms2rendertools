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

namespace GraphicsEngine
{
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

			auto reference = object->Reference.lock();

			if (!reference->IsTransparent())
				Lights[lightIndex].Watch.Insert(object, reference->GetBoundingBox());
		});

		DynamicObjects.PairQuery(UpdatedLights, [this] (const AabbTree::Node* objectNode, const AabbTree::Node* lightNode)
		{
			SceneObjectReference* object = objectNode->GetData<SceneObjectReference>();
			int lightIndex = int(lightNode->ClientData);

			auto reference = object->Reference.lock();

			if (!reference->IsTransparent())
				Lights[lightIndex].Watch.Insert(object, reference->GetBoundingBox());
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
		std::queue<SceneRayCastResults> hits;

		auto resultsProcessorLambda = [&hits, &ray, &callback] (const AabbTree::Node* node, float t)
		{
			SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

			objectReference->Reference.lock()->CastRay(ray, [&hits, &callback] (const SceneRayCastResults& results)
			{
				//callback(results);
				hits.push(results);
			});
		};

		StaticObjects.CastRay(ray, std::ref(resultsProcessorLambda));

		DynamicObjects.CastRay(ray, std::ref(resultsProcessorLambda));

		while (!hits.empty())
		{
			callback(hits.front());
		
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

	std::shared_ptr<SceneObject> Scene::GetStaticObject(const AabbTree::Node* node) const
	{
		SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

		return objectReference->Reference.lock();
	}

	std::shared_ptr<SceneObject> Scene::GetDynamicObject(const AabbTree::Node* node) const
	{
		SceneObjectReference* objectReference = node->GetData<SceneObjectReference>();

		return objectReference->Reference.lock();
	}

	void Scene::ClearStaticObjects()
	{
		StaticObjects.Clear();

		for (int i = 0; i < int(StaticObjectReferences.size()); ++i)
			StaticObjectHandles.Destroy(StaticObjectReferences[i]);

		StaticObjectReferences.clear();
	}

	void Scene::Update(float)
	{
		Updating = true;

		{
			UpdateVector.clear();

			StaticObjects.VisitAll([this] (const AabbTree::Node* node)
			{
				SceneObjectReference* object = node->GetData<SceneObjectReference>();

				if (object->Reference.lock()->HasMoved())
					Update(object->ID);
			});

			const AabbTree& objects = StaticObjects;

			for (int i = 0; i < int(UpdateVector.size()); ++i)
				StaticObjects.Update(
					UpdateVector[i],
					objects.GetNode(UpdateVector[i])->GetData<SceneObjectReference>()->Reference.lock()->GetBoundingBox()
				);
		}

		{
			UpdateVector.clear();

			DynamicObjects.VisitAll([this] (const AabbTree::Node* node)
			{
				SceneObjectReference* object = node->GetData<SceneObjectReference>();

				if (object->Reference.lock()->HasMoved())
					Update(object->ID);
			});

			const AabbTree& objects = DynamicObjects;

			for (int i = 0; i < int(UpdateVector.size()); ++i)
				DynamicObjects.Update(
					UpdateVector[i],
					objects.GetNode(UpdateVector[i])->GetData<SceneObjectReference>()->Reference.lock()->GetBoundingBox()
				);
		}

		Updating = false;
	}

	void Scene::AddObject(const std::shared_ptr<SceneObject>& object)
	{
		if (object->IsStatic())
		{
			SceneObjectReference* handle = StaticObjectHandles.Create<SceneObjectReference>(This.lock()->Cast<Object>(), object);

			handle->ID = StaticObjects.Insert(handle, object->GetBoundingBox());

			StaticObjectReferences.push_back(handle);
		}
		else
		{
			SceneObjectReference* handle = StaticObjectHandles.Create<SceneObjectReference>(This.lock()->Cast<Object>(), object);

			handle->ID = DynamicObjects.Insert(handle, object->GetBoundingBox());

			DynamicObjectReferences.push_back(handle);
			//Objects.push_back(object);
			//
			//Objects[Objects.size() - 1].SetParent(This);
			//Objects[Objects.size() - 1] = object;
		}
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
		if (object->IsStatic())
		{
			int i = 0;

			for (i = 0; i < int(StaticObjectReferences.size()) && StaticObjectReferences[i]->Reference.lock() != object; ++i);

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

			for (i = 0; i < int(DynamicObjectReferences.size()) && DynamicObjectReferences[i]->Reference.lock() != object; ++i);

			if (i < int(DynamicObjectReferences.size()))
			{
				DynamicObjects.Remove(DynamicObjectReferences[i]->ID);

				StaticObjectHandles.Destroy(DynamicObjectReferences[i]);

				std::swap(DynamicObjectReferences[i], DynamicObjectReferences.back());

				DynamicObjectReferences.pop_back();
			}
		}
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

			if (object->Visible && object->IsTransparent() == drawTransparent && !object->MaterialProperties.expired())
			{
				if (drawTransparent)
					Programs::PhongForward->SetMaterial(object->MaterialProperties.lock());
				else
					Programs::Phong->SetMaterial(object->MaterialProperties.lock());

				object->Draw(camera);
			}
		}

		Draw(StaticObjects, drawTransparent, camera);
		Draw(DynamicObjects, drawTransparent, camera);

		//StaticObjects.CastFrustum(camera->GetFrustum(), [drawTransparent, &camera] (const AabbTree::Node* node)
		//{
		//	SceneObjectReference* thingamajig = node->GetData<SceneObjectReference>();
		//	//WeakHandle<SceneObject> object = thingamajig->Reference;
		//
		//	if (thingamajig->Reference->Visible && thingamajig->Reference->IsTransparent() == drawTransparent && !thingamajig->Reference->MaterialProperties.IsNull())
		//	{
		//		if (drawTransparent)
		//			Programs::PhongForward->SetMaterial(thingamajig->Reference->MaterialProperties);
		//		else
		//			Programs::Phong->SetMaterial(thingamajig->Reference->MaterialProperties);
		//
		//		thingamajig->Reference->Draw(camera);
		//	}
		//});
	}

	void Scene::Draw(const AabbTree& tree, bool drawTransparent, const std::shared_ptr<Camera>& targetCamera)
	{
		int drawn = 0;
		tree.CastFrustum(targetCamera->GetFrustum(), [drawTransparent, &targetCamera, &drawn] (const AabbTree::Node* node)
		{
			SceneObjectReference* thingamajig = node->GetData<SceneObjectReference>();
			//WeakHandle<SceneObject> object = thingamajig->Reference;

			auto reference = thingamajig->Reference.lock();

			if (reference->Visible && reference->IsTransparent() == drawTransparent && !reference->MaterialProperties.expired())
			{
				++drawn;

				if (drawTransparent && ShaderProgram::GetCurrentProgram() == Programs::PhongForward)
					Programs::PhongForward->SetMaterial(reference->MaterialProperties.lock());
				else if (ShaderProgram::GetCurrentProgram() == Programs::Phong)
					Programs::Phong->SetMaterial(reference->MaterialProperties.lock());

				reference->Draw(targetCamera);
			}
		});

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

	std::shared_ptr<Camera>& Scene::Target()
	{
		static std::shared_ptr<Camera> camera = nullptr;

		return camera;
	}
}
