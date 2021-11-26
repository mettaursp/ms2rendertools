#include "RayAcceleration.h"

#include "Vector3.h"
#include "Scene.h"
#include "AabbTree.h"
#include "Model.h"
#include "PhysicalMaterial.h"
#include "MeshData.h"
#include "ModelAsset.h"
#include "MeshLoader.h"
#include "Transform.h"

namespace GraphicsEngine
{
	//bool rayAabbIntersect(const RayAcceleration::FastRay& ray, const RayAcceleration::Aabb& box)
	//{
	//	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	//
	//	tmin = (box.Bounds[ray.Sign[0]].X - ray.Start.X) * ray.InverseDirection.X;
	//	tmax = (box.Bounds[1 - ray.Sign[0]].X - ray.Start.X) * ray.InverseDirection.X;
	//	tymin = (box.Bounds[ray.Sign[1]].Y - ray.Start.Y) * ray.InverseDirection.Y;
	//	tymax = (box.Bounds[1 - ray.Sign[1]].Y - ray.Start.Y) * ray.InverseDirection.Y;
	//
	//	if ((tmin > tymax) || (tymin > tmax))
	//		return false;
	//	if (tymin > tmin)
	//		tmin = tymin;
	//	if (tymax < tmax)
	//		tmax = tymax;
	//
	//	tzmin = (box.Bounds[ray.Sign[2]].Z - ray.Start.Z) * ray.InverseDirection.Z;
	//	tzmax = (box.Bounds[1 - ray.Sign[2]].Z - ray.Start.Z) * ray.InverseDirection.Z;
	//
	//	if ((tmin > tzmax) || (tzmin > tmax))
	//		return false;
	//	if (tzmin > tmin)
	//		tmin = tzmin;
	//	if (tzmax < tmax)
	//		tmax = tzmax;
	//
	//	return true;
	//}

	Vector transform(const RayAcceleration::Matrix& matrix, const Vector& vector)
	{
		return Vector{
			vector.X * matrix.Data[0][0] + vector.Y * matrix.Data[0][1] + vector.Z * matrix.Data[0][2],
			vector.X * matrix.Data[1][0] + vector.Y * matrix.Data[1][1] + vector.Z * matrix.Data[1][2],
			vector.X * matrix.Data[2][0] + vector.Y * matrix.Data[2][1] + vector.Z * matrix.Data[2][2]
		};
	}

	Vector add(const Vector& vector, const Vector& vector2)
	{
		return Vector{ vector.X + vector2.X, vector.Y + vector2.Y, vector.Z + vector2.Z };
	}

	BoundingBox box(const Aabb& input)
	{
		return BoundingBox{ {
			{ input.Min.X, input.Min.Y, input.Min.Z },
			{ input.Max.X, input.Max.Y, input.Max.Z }
		} };
	}

	void RayAcceleration::Accelerate(const std::shared_ptr<Scene>& scene)
	{
		Materials.clear();
		Meshes.clear();
		Nodes.clear();
		Objects.clear();
		Indices.clear();
		Vertices.clear();
		Faces.clear();

		ObjectTreeHead = -1;
		LightTreeHead = -1;

		MeshLeftNodes = 0;
		MeshRightNodes = 0;

		int staticObjectCount = scene->GetStaticObjectCount();
		int dynamicObjectCount = scene->GetDynamicbjectCount();

		const AabbTree& staticObjects = scene->GetStaticObjects();
		const AabbTree& dynamicObjects = scene->GetDynamicObjects();

		int nodeCount = staticObjects.Size() + dynamicObjects.Size();
		bool mergeTrees = staticObjects.Size() > 0 && dynamicObjects.Size() > 0;

		if (mergeTrees)
			++nodeCount;

		if (staticObjectCount + dynamicObjectCount > 0)
		{
			ObjectTreeHead = 0;

			Objects.reserve(staticObjectCount + dynamicObjectCount);
			Nodes.reserve(nodeCount);

			::Aabb bounding;

			const AabbTree::Node* staticRoot = staticObjects.GetRoot();
			const AabbTree::Node* dynamicRoot = dynamicObjects.GetRoot();

			if (mergeTrees)
			{
				if (staticRoot != nullptr)
					bounding.Expand(staticRoot->Box.Min).Expand(staticRoot->Box.Max);

				if (dynamicRoot != nullptr)
					bounding.Expand(dynamicRoot->Box.Min).Expand(dynamicRoot->Box.Max);

				Nodes.push_back(TreeNode{ -1, 1, 0, box(bounding) });
			}

			if (staticRoot != nullptr)
				PushNode(staticRoot, false, scene);

			if (mergeTrees)
				Nodes[0].Right = int(Nodes.size());

			if (dynamicRoot != nullptr)
				PushNode(dynamicRoot, false, scene);
		}

		for (int i = 0; i < int(Meshes.size()); ++i)
		{
			Meshes[i].TreeHead = int(Nodes.size());

			PushMeshNode(MeshLoader::GetMeshData(MeshIds[i])->GetTree().GetRoot(), i);
		}

		std::cout << "left nodes: " << MeshLeftNodes << std::endl;
		std::cout << "right nodes: " << MeshRightNodes << std::endl;

		Lights.reserve(scene->GetLights() + 1);

		for (int i = 0; i < scene->GetLights(); ++i)
		{
			std::shared_ptr<GraphicsEngine::Light> source = scene->GetLight(i);

			if (source->Enabled)
			{
				if (source->Type == Enum::LightType::Directional)
					DirectionalLights.push_back(int(Lights.size()));

				Lights.push_back(Light{
					255 * source->Brightness * source->GetRadius() * std::max(std::max(source->Diffuse.R, source->Diffuse.G), source->Diffuse.B),
					source->Brightness,
					source->GetRadius(),
					std::cosf(source->InnerRadius),
					std::cosf(source->OuterRadius),
					source->GetAttenuationOffset(),
					source->SpotlightFalloff,
					source->Type,
					source->Attenuation,
					source->Position,
					source->Direction,
					source->Diffuse
				});
			}
		}

		if (!scene->GlobalLight.expired())
		{
			std::shared_ptr<GraphicsEngine::Light> source = scene->GlobalLight.lock();

			Lights.push_back(Light{
				255 * source->Brightness * source->GetRadius() * std::max(std::max(source->Diffuse.R, source->Diffuse.G), source->Diffuse.B),
				source->Brightness,
				source->GetRadius(),
				source->InnerRadius,
				source->OuterRadius,
				source->GetAttenuationOffset(),
				source->SpotlightFalloff,
				source->Type,
				source->Attenuation,
				source->Position,
				source->Direction,
				source->Diffuse
			});

			GlobalLight = &Lights.back();
		}
	}

	Vector vec(const Vector3& vector)
	{
		return Vector{ vector.X, vector.Y, vector.Z};
	}

	RayAcceleration::Matrix mat(const Matrix3& matrix)
	{
		return RayAcceleration::Matrix{ {
			{ matrix.Data[0][0], matrix.Data[0][1], matrix.Data[0][2] },
			{ matrix.Data[1][0], matrix.Data[1][1], matrix.Data[1][2] },
			{ matrix.Data[2][0], matrix.Data[2][1], matrix.Data[2][2] }
		} };
	}

	void RayAcceleration::PushNode(const AabbTree::Node* node, bool isDynamic, const std::shared_ptr<Scene>& scene)
	{
		int objectIndex = -1;

		if (node->IsLeaf)
		{
			SceneObject* object = nullptr;

			if (isDynamic)
				object = scene->GetDynamicObject(node);
			else
				object = scene->GetStaticObject(node);

			if (object->IsA<Model>())
			{
				std::shared_ptr<Model> model = object->Cast<Model>();
				std::shared_ptr<Engine::Transform> transform = model->GetComponent<Engine::Transform>();

				if (!(model->Asset.expired() || model->GetMaterialRaw() == nullptr) && transform != nullptr)
				{
					objectIndex = int(Objects.size());

					Objects.push_back(Object{
						PushMesh(MeshLoader::GetMeshData(model->Asset.lock()->GetMeshID()), model->Asset.lock()->GetMeshID(), model->Asset.lock()->Name),
						PushMaterial(model->PhysicalMaterialProperties.lock()),
						model->Color,
						model->GlowColor,
						vec(transform->GetWorldTransformation().Translation()),
						vec(transform->GetWorldTransformationInverse().Translation()),
						mat(transform->GetWorldTransformation()),
						mat(transform->GetWorldNormalTransformation()),
						mat(transform->GetWorldTransformationInverse())
					});
				}
			}
		}

		int index = int(Nodes.size());

		Nodes.push_back(TreeNode{ objectIndex, node->GetLeft() != nullptr ? index + 1 : -1, -1, box(node->Box) });

		if (node->GetLeft() != nullptr)
			PushNode(node->GetLeft(), isDynamic, scene);

		if (node->GetRight() != nullptr)
		{
			Nodes[index].Right = int(Nodes.size());

			PushNode(node->GetRight(), isDynamic, scene);
		}
	}

	int RayAcceleration::PushMesh(const MeshData* data, int id, const std::string& name)
	{
		if (data->DrawMode != Enum::VertexMode::Wireframe)
		{
			char* pointer = nullptr;
			pointer += 1000 + id;

			IndexMap::iterator i = Indices.find(pointer);

			if (i != Indices.end())
				return i->second;

			int index = int(Meshes.size());

			Indices[pointer] = index;

			Meshes.push_back(Mesh{
				BoundingBox { data->GetMinimumCorner(), data->GetMaximumCorner() },
				int(Faces.size()),
				int(data->IndexBuffer.size()) / 3
			});
			MeshIds.push_back(id);

			int startIndex = int(Vertices.size());

			for (int i = 0; i < int(data->VertexBuffer.size()); ++i)
			{
				Vertices.push_back(Vertex{
					vec(data->VertexBuffer[i].Position),
					Vector3(data->VertexBuffer[i].Normal).Unit()
				});
			}

			int removedFaces = 0;

			for (int i = 0; i < int(data->IndexBuffer.size()); i += 3)
			{
				int a = data->IndexBuffer[i];
				int b = data->IndexBuffer[i + 1];
				int c = data->IndexBuffer[i + 2];

				Vector3 normal = Vector3(data->VertexBuffer[b].Position - data->VertexBuffer[a].Position).Cross(data->VertexBuffer[c].Position - data->VertexBuffer[a].Position);
				float length = normal.SquareLength();

				if (length <= 1e-9)
				{
					std::cout << "degenerate triangle in " << id << " at " << (i / 3) << std::endl;
					++removedFaces;
					//
					//continue;
				}

				//normal *= 1 / std::sqrtf(length);

				Faces.push_back(Face{ startIndex + a, startIndex + b, startIndex + c, 1 / length, vec(normal) });
			}

			if (removedFaces > 0)
				std::cout << removedFaces << " degenerate triangles found in mesh " << name << "[" << id << "]" << std::endl;

			Meshes.back().TriangleCount -= removedFaces;

			Meshes.back().KDTreeHead = AcceleratedKDTree::Construct(Meshes.back(), KDNodes, Vertices, Faces, Nodes);

			return index;
		}

		return -1;
	}

	void RayAcceleration::PushMeshNode(const AabbTree::Node* node, int meshIndex)
	{
		if (node == nullptr)
			return;

		int objectIndex = -1;

		if (node->IsLeaf)
			objectIndex = Meshes[meshIndex].StartIndex + MeshLoader::GetMeshData(MeshIds[meshIndex])->GetTriangle(node);

		int index = int(Nodes.size());

		Nodes.push_back(TreeNode{ objectIndex, node->GetLeft() != nullptr ? index + 1 : -1, -1, box(node->Box) });

		if (node->GetLeft() != nullptr)
		{
			++MeshLeftNodes;
			PushMeshNode(node->GetLeft(), meshIndex);
		}

		if (node->GetRight() != nullptr)
		{
			++MeshRightNodes;
			Nodes[index].Right = int(Nodes.size());

			PushMeshNode(node->GetRight(), meshIndex);
		}
	}

	void RayAcceleration::UpdateMaterials()
	{
		for (MaterialMap::iterator i = MaterialCache.begin(); i != MaterialCache.end(); ++i)
		{
			GraphicsEngine::PhysicalMaterial* material = i->first;

			Materials[i->second] = Material{
				material->Albedo,
				material->Roughness,
				material->Metalness,
				material->RefractiveIndex,
				material->Transparency,
				material->Translucency,
				material->Emission
			};
		}
	}

	int RayAcceleration::PushMaterial(const std::shared_ptr<GraphicsEngine::PhysicalMaterial>& material)
	{
		IndexMap::iterator i = Indices.find(&*material);

		if (i != Indices.end())
			return i->second;

		int index = int(Materials.size());

		Indices[&*material] = index;
		MaterialCache[&*material] = index;

		Materials.push_back(Material{
			material->Albedo,
			material->Roughness,
			material->Metalness,
			material->RefractiveIndex,
			material->Transparency,
			material->Translucency,
			material->Emission
		});

		return index;
	}

	void RayAcceleration::CastRay(const Ray& ray, float length, const ResultsCallback& callback, const ResultsCallback& filter, IndexVector& stack, int startObjectNode, int startNode) const
	{
		float farthest = length;
		const Object* object = nullptr;
		Ray localRay;
		int objectID = -1;
		int currentObjectNode = -1;

		auto processTriangles = [this, &farthest, &localRay, &object, &objectID, &callback, &filter, &currentObjectNode](int objectIndex, int objectNode)
		{
			const Face& face = Faces[objectIndex];

			Vector3& rayStart = localRay.Start;
			Vector3& rayDirection = localRay.Direction;

			const Vector& vertexARaw = Vertices[face.A].Position;
			const Vector& vertexBRaw = Vertices[face.B].Position;
			const Vector& vertexCRaw = Vertices[face.C].Position;

			Vector3 vertexA = Vector3(vertexARaw.X, vertexARaw.Y, vertexARaw.Z, 1);
			Vector3 vertexB = Vector3(vertexBRaw.X, vertexBRaw.Y, vertexBRaw.Z, 1);
			Vector3 vertexC = Vector3(vertexCRaw.X, vertexCRaw.Y, vertexCRaw.Z, 1);

			Vector3 normal(face.Normal.X, face.Normal.Y, face.Normal.Z);

			float normalDot = normal * Vector3(localRay.Direction.X, localRay.Direction.Y, localRay.Direction.Z);

			float distance = (normal * (vertexA - rayStart)) / normalDot;

			if (distance > farthest || distance < 0)
				return;

			Vector3 intersection = rayStart + distance * rayDirection;

			float dot1 = Vector3(vertexB - vertexA).Cross(intersection - vertexA).Dot(normal);
			float dot2 = Vector3(vertexC - vertexB).Cross(intersection - vertexB).Dot(normal);
			float dot3 = Vector3(vertexA - vertexC).Cross(intersection - vertexC).Dot(normal);

			if (!(std::signbit(dot1) == std::signbit(dot2) && std::signbit(dot1) == std::signbit(dot3)))
				//if (!(std::signbit(dot1) & std::signbit(dot2) & std::signbit(dot3)))
				return;

			const Material* material = &Materials[object->MaterialProperties];

			CastResults results{ normalDot < 0, distance, material, objectID, currentObjectNode, objectNode, objectIndex, Vector3(object->Color.X, object->Color.Y, object->Color.Z, 1 - material->Transparency) };

			RayFilterResults filterResults = filter(results);

			if (filterResults == RayFilterResults::Stop)
				farthest = -1;

			if (filterResults != RayFilterResults::Confirm)
				return;

			float u = dot2 * face.Area;
			float v = dot3 * face.Area;
			float w = 1 - u - v;

			results.BaryCentric = Vector(u, v, w);

			Vector faceNormal = Vector(u * Vector3(Vertices[face.A].Normal) + v * Vector3(Vertices[face.B].Normal) + w * Vector3(Vertices[face.C].Normal));
			Vector normalData = transform(object->NormalTransformation, faceNormal);
			Vector intersectionData = add(object->Position, transform(object->Transformation, vec(intersection)));

			results.Normal = Vector3(normalData.X, normalData.Y, normalData.Z);
			results.Intersection = Vector3(intersectionData.X, intersectionData.Y, intersectionData.Z, 1);

			filterResults = callback(results);

			switch (filterResults)
			{
			case RayFilterResults::Ignore: return;
			case RayFilterResults::Confirm: farthest = distance; break;
			case RayFilterResults::Stop: farthest = -1;
			}
		};

		auto processStartObject = [this, &ray, &farthest, &object, &localRay, &objectID, &processTriangles, &startNode, &stack](int objectIndex, int objectNode)
		{
			object = &Objects[objectIndex];

			if (object->Mesh == -1)
				return;

			const Mesh& mesh = Meshes[object->Mesh];

			if (mesh.TreeHead == -1)
				return;

			localRay = Ray(Vector3(add(object->InverseOffset, transform(object->InverseTransformation, ray.Start))) + Vector3(0, 0, 0, 1), transform(object->InverseTransformation, ray.Direction));

			objectID = objectIndex;

			if (startNode != -1)
				CastRay(startNode, localRay, farthest, std::ref(processTriangles), stack);

			if (farthest == 1)
				CastRay(mesh.TreeHead, localRay, farthest, std::ref(processTriangles), stack);
		};

		auto processObjects = [this, &ray, &farthest, &object, &localRay, &objectID, &processTriangles, &currentObjectNode, &stack](int objectIndex, int objectNode = -1)
		{
			object = &Objects[objectIndex];

			if (object->Mesh == -1)
				return;

			const Mesh& mesh = Meshes[object->Mesh];

			if (mesh.TreeHead == -1)
				return;

			localRay = Ray(Vector3(add(object->InverseOffset, transform(object->InverseTransformation, ray.Start))) + Vector3(0, 0, 0, 1), transform(object->InverseTransformation, ray.Direction));

			objectID = objectIndex;
			currentObjectNode = objectNode;

			CastRay(mesh.TreeHead, localRay, farthest, std::ref(processTriangles), stack);
		};

		stack.clear();

		if (startObjectNode != -1)
			CastRay(startObjectNode, ray, farthest, std::ref(processObjects), stack);

		if (ObjectTreeHead != -1)
			CastRay(ObjectTreeHead, ray, farthest, std::ref(processObjects), stack);
	}

	const float infinity = std::numeric_limits<float>::max();

	bool IntersectsAxis(float start, float direction, float min, float max, float& tMin, float& tMax)
	{
		if (direction > -1e5f || direction < 1e5f)
		{
			float t1 = (min - start) * direction;
			float t2 = (max - start) * direction;

			if (t1 > t2)
				std::swap(t1, t2);

			if (t1 > t2)
			{
				tMin = std::max(tMin, t2);
				tMax = std::min(tMax, t1);
			}
			else
			{
				tMin = std::max(tMin, t1);
				tMax = std::min(tMax, t2);
			}
		}
		else
			return start >= min && start <= max;

		return tMin <= tMax;
	}

	bool RayIntersects(const RayAcceleration::FastRay& ray, const Aabb& box, float& t)
	{
		float tMax = infinity;
		float tMin = -tMax;

		if (!(
			IntersectsAxis(ray.Start.X, ray.InverseDirection.X, box.Min.X, box.Max.X, tMin, tMax) &&
			IntersectsAxis(ray.Start.Y, ray.InverseDirection.Y, box.Min.Y, box.Max.Y, tMin, tMax) &&
			IntersectsAxis(ray.Start.Z, ray.InverseDirection.Z, box.Min.Z, box.Max.Z, tMin, tMax)
			) || tMax < 0)
			return false;

		t = std::max(0.f, tMin);

		return true;
	}

	bool RayIntersects(const Ray& ray, const Aabb& box, float& t)
	{
		float tMax = infinity;
		float tMin = 0;

		if (!(
			IntersectsAxis(ray.Start.X, 1 / ray.Direction.X, box.Min.X, box.Max.X, tMin, tMax) &&
			IntersectsAxis(ray.Start.Y, 1 / ray.Direction.Y, box.Min.Y, box.Max.Y, tMin, tMax) &&
			IntersectsAxis(ray.Start.Z, 1 / ray.Direction.Z, box.Min.Z, box.Max.Z, tMin, tMax)
			) || tMax < 0)
			return false;

		t = std::max(0.f, tMin);

		return true;
	}

	void RayAcceleration::CastRay(int node, const Ray& ray, float& farthest, const ObjectCallback& callback, IndexVector& stack) const
	{
		size_t startSize = stack.size();

		stack.push_back(-1);

		auto pop = [&node, &stack]()
		{
			node = stack.back();
			stack.pop_back();
		};

		auto processNode = [&node, this, &farthest, &ray, &callback, &pop, &stack]()
		{
			const TreeNode& treeNode = Nodes[node];

			float distance = infinity;

			if (farthest > 0 && RayIntersects(ray, ::Aabb(treeNode.Box.Bounds[0], treeNode.Box.Bounds[1]), distance))
			{
				if (distance > farthest)
					return pop();

				if (treeNode.Index != -1)
				{
					callback(treeNode.Index, node);

					return pop();
				}

				if (treeNode.Left == -1 && treeNode.Right == -1)
					return pop();

				if (treeNode.Left != -1)
				{
					node = treeNode.Left;

					if (treeNode.Right != -1)
						stack.push_back(treeNode.Right);
				}
				else
					node = treeNode.Right;
			}
			else
				return pop();
		};

		int nodeTests = 0;

		while (node != -1) {
			processNode();
			++nodeTests;
		}
	}
}