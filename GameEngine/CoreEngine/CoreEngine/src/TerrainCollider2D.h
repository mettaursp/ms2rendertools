#pragma once

#include "Object.h"
#include "AabbTree.h"
#include "TerrainColliderAxisEnum.h"
#include "Coordinates.h"
#include "Collider2D.h"

namespace Engine
{
	class Chunk;
	class ChunkTree;
	class ChunkColumn;
	class Terrain;
	class Transform;

	namespace Physics
	{
		class Collider2D;
		class ColliderData;
	}

	class TerrainCollider2D : public Physics::Collider2D
	{
	public:
		typedef std::function<void(const std::shared_ptr<Physics::Collider2D>& collider, const Physics::ColliderData& cell)> TerrainPairCallback;
		typedef std::function<std::shared_ptr<Physics::Collider2D>(void*)> ColliderCallback;

		Enum::TerrainColliderAxis Axis = Enum::TerrainColliderAxis::XYPlane;

		void Initialize() {}
		void Update(float) {}

		void PairQuery(const AabbTree::Node* head, const TerrainPairCallback& collisionCallback, const ColliderCallback& lookupCallback);

		void InitializeReflection();

	private:
		struct ChildComparatorEnum
		{
			enum ChildComparator
			{
				Neither,

				Left,
				Right,

				Both,
				LeafCollision
			};
		};

		struct QueryData
		{
			const TerrainPairCallback& CollisionPairCallback;
			const ColliderCallback& NodeColliderCallback;
			const std::shared_ptr<Terrain>& CurrentTerrain;
			const std::shared_ptr<Transform>& CurrentTransform;
			Aabb CurrentNodeBox;
			Aabb LeftNodeBox;
			Aabb RightNodeBox;
			Aabb RegionBox;
			Coordinates Index;
			float Occupancy[4];
			Physics::ColliderData& Collider;

			QueryData(const std::shared_ptr<Terrain>& terrain, const std::shared_ptr<Transform>& transform, Physics::ColliderData& collider, const TerrainPairCallback& collisionPairCallback, const ColliderCallback& nodeColliderCallback) : CurrentTerrain(terrain), CurrentTransform(transform), Collider(collider), CollisionPairCallback(collisionPairCallback), NodeColliderCallback(nodeColliderCallback) {}
		};
		
		typedef ChildComparatorEnum::ChildComparator ChildComparator;

		Aabb GetChunkBoundingBox(const std::shared_ptr<Chunk>& chunk) const;

		Aabb GetNodeBoundingBox(const AabbTree::Node* node) const;

		ChildComparator GetIntersection(QueryData& query, const AabbTree::Node* node) const;
		
		void ComputeNodeBoxes(QueryData& query, const AabbTree::Node* node) const;
		void Visit(QueryData& query, const AabbTree::Node* node, const ChunkTree* chunks) const;
		void Visit(QueryData& query, const AabbTree::Node* node, const ChunkColumn* chunks) const;
		void Visit(QueryData& query, const AabbTree::Node* node, const std::shared_ptr<Chunk>& chunk) const;
		bool GetCellCollider(QueryData& query, const std::shared_ptr<Chunk>& chunk, int x, int y, int z) const;
		void FillOccupancy(float* occupancy, const std::shared_ptr<Chunk>& chunk, int x, int y, int z) const;
		int GetPermutation(const float* occupancy) const;
		float FetchOccupancy(const std::shared_ptr<Chunk>* chunks, int x, int y, int z) const;

		Instantiable;

		Inherits_Class(Physics::Collider2D);

		Reflected(TerrainCollider2D);
	};
}