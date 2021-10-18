#include "TerrainCollider2D.h"

#include <algorithm>

#include "ColliderData.h"
#include "ChunkTree.h"
#include "Chunk.h"
#include "Terrain.h"
#include "VoxelTriangulation.h"
#include "Transform.h"

namespace Engine
{
	void TerrainCollider2D::PairQuery(const AabbTree::Node* head, const TerrainPairCallback& collisionCallback, const ColliderCallback& lookupCallback)
	{
		if (head == nullptr)
			return;

		Physics::ColliderData collider;
		std::shared_ptr<Terrain> terrain = GetComponent<Terrain>();
		std::shared_ptr<Transform> transform = GetComponent<Transform>();

		if (terrain == nullptr || transform == nullptr)
			return;

		QueryData query(terrain, transform, collider, collisionCallback, lookupCallback);

		Visit(query, head, terrain->GetHead());
	}

	Aabb TerrainCollider2D::GetChunkBoundingBox(const std::shared_ptr<Chunk>& chunk) const
	{
		Coordinates index = chunk->GetCoordinates();

		index.X *= 16;
		index.Y *= 16;
		index.Z *= 16;

		return Aabb(
			Vector3(index),
			Vector3(index) + Vector3(16, 16, 16)
		);
	}

	Aabb TerrainCollider2D::GetNodeBoundingBox(const AabbTree::Node* node) const
	{
		Aabb box = node->Box;

		float cellZ = std::floorf(0.5f * (box.Min.Z + box.Max.Z));

		box.Min.Z = cellZ + 0.1f;
		box.Max.Z = cellZ + 0.9f;

		switch (Axis)
		{
		case Enum::TerrainColliderAxis::XYPlane: break;
		case Enum::TerrainColliderAxis::XZPlane:
			std::swap(box.Min.Y, box.Min.Z);
			std::swap(box.Max.Y, box.Max.Z);

			break;
		case Enum::TerrainColliderAxis::ZYPlane:
			std::swap(box.Min.X, box.Min.Z);
			std::swap(box.Max.X, box.Max.Z);

			break;
		case Enum::TerrainColliderAxis::YXPlane:break;
		case Enum::TerrainColliderAxis::ZXPlane:
			std::swap(box.Min.Y, box.Min.Z);
			std::swap(box.Max.Y, box.Max.Z);

			break;
		case Enum::TerrainColliderAxis::YZPlane:
			std::swap(box.Min.X, box.Min.Z);
			std::swap(box.Max.X, box.Max.Z);

			break;
		}

		if (Axis > Enum::TerrainColliderAxis::YXPlane)
		{
			std::swap(box.Min.X, box.Min.Y);
			std::swap(box.Max.X, box.Max.Y);
		}

		return box;
	}

	TerrainCollider2D::ChildComparator TerrainCollider2D::GetIntersection(QueryData& query, const AabbTree::Node* node) const
	{
		if (node == nullptr || !query.CurrentNodeBox.Intersects(query.RegionBox))
			return ChildComparator::Neither;

		if (node->IsLeaf)
			return ChildComparator::LeafCollision;

		ChildComparator left = ChildComparator::Neither;
		ChildComparator right = ChildComparator::Neither;

		const AabbTree::Node* leftNode = node->GetLeft();
		const AabbTree::Node* rightNode = node->GetRight();

		if (leftNode != nullptr && query.LeftNodeBox.Intersects(query.RegionBox))
			left = ChildComparator::Left;

		if (rightNode != nullptr && query.RightNodeBox.Intersects(query.RegionBox))
			right = ChildComparator::Right;

		return ChildComparator(left | right);
	}

	void TerrainCollider2D::ComputeNodeBoxes(QueryData& query, const AabbTree::Node* node) const
	{
		query.CurrentNodeBox = GetNodeBoundingBox(node);

		const AabbTree::Node* leftNode = node->GetLeft();
		const AabbTree::Node* rightNode = node->GetRight();

		if (leftNode != nullptr)
			query.LeftNodeBox = GetNodeBoundingBox(leftNode);

		if (rightNode != nullptr)
			query.RightNodeBox = GetNodeBoundingBox(rightNode);
	}

	Aabb getIntersection(const Aabb& box1, const Aabb& box2)
	{
		Aabb box;

		box.Min.X = std::max(box1.Min.X, box2.Min.X);
		box.Min.Y = std::max(box1.Min.Y, box2.Min.Y);
		box.Min.Z = std::max(box1.Min.Z, box2.Min.Z);

		box.Max.X = std::min(box1.Max.X, box2.Max.X);
		box.Max.Y = std::min(box1.Max.Y, box2.Max.Y);
		box.Max.Z = std::min(box1.Max.Z, box2.Max.Z);

		return box;
	}

	void TerrainCollider2D::Visit(QueryData& query, const AabbTree::Node* node, const ChunkTree* chunks) const
	{
		if (chunks == nullptr)
			return;

		query.RegionBox = chunks->GetBoundingBox();

		ComputeNodeBoxes(query, node);

		switch (GetIntersection(query, node))
		{
		case ChildComparator::Neither: return;
		case ChildComparator::Left: return Visit(query, node->GetLeft(), chunks);
		case ChildComparator::Right: return Visit(query, node->GetRight(), chunks);
		case ChildComparator::Both:
		case ChildComparator::LeafCollision: break;
		}

		if (chunks->IsLeaf())
			return Visit(query, node, chunks->GetColumn());

		Aabb box = getIntersection(query.RegionBox, query.CurrentNodeBox);

		box.Max -= Vector3(0.05f, 0.05f, 0.05f);

		Coordinates min = chunks->GetChildIndex(query.CurrentTerrain->GetChunkCoordinates(box.Min));
		Coordinates max = chunks->GetChildIndex(query.CurrentTerrain->GetChunkCoordinates(box.Max));

		for (int x = min.X; x <= max.X; ++x)
			for (int z = min.Y; z <= max.Y; ++z)
				Visit(query, node, chunks->GetChild(Coordinates(x, 0, z)));
	}

	void TerrainCollider2D::Visit(QueryData& query, const AabbTree::Node* node, const ChunkColumn* chunks) const
	{
		if (chunks == nullptr)
			return;

		query.RegionBox = chunks->GetBoundingBox();

		ComputeNodeBoxes(query, node);

		switch (GetIntersection(query, node))
		{
		case ChildComparator::Neither: return;
		case ChildComparator::Left: return Visit(query, node->GetLeft(), chunks);
		case ChildComparator::Right: return Visit(query, node->GetRight(), chunks);
		case ChildComparator::Both:
		case ChildComparator::LeafCollision: break;
		}

		if (chunks->IsLeaf())
			return Visit(query, node, chunks->GetChunk());

		Aabb box = getIntersection(query.RegionBox, query.CurrentNodeBox);

		box.Max -= Vector3(0.05f, 0.05f, 0.05f);

		int min = chunks->GetChildIndex(query.CurrentTerrain->GetChunkCoordinates(box.Min));
		int max = chunks->GetChildIndex(query.CurrentTerrain->GetChunkCoordinates(box.Max));

		for (int y = min; y <= max; ++y)
			Visit(query, node, chunks->GetChild(y));
	}

	void TerrainCollider2D::Visit(QueryData& query, const AabbTree::Node* node, const std::shared_ptr<Chunk>& chunk) const
	{
		if (node == nullptr)
			return;

		query.RegionBox = GetChunkBoundingBox(chunk);

		ComputeNodeBoxes(query, node);

		switch (GetIntersection(query, node))
		{
		case ChildComparator::Neither: return;
		case ChildComparator::Left: return Visit(query, node->GetLeft(), chunk);
		case ChildComparator::Right: return Visit(query, node->GetRight(), chunk);
		case ChildComparator::Both:
			Visit(query, node->GetLeft(), chunk);
			Visit(query, node->GetRight(), chunk);

			return;
		case ChildComparator::LeafCollision: break;
		}

		Aabb box = getIntersection(query.RegionBox, query.CurrentNodeBox);

		box.Max -= Vector3(0.05f, 0.05f, 0.05f);

		Coordinates min = query.CurrentTerrain->GetCellCoordinates(box.Min);
		Coordinates max = query.CurrentTerrain->GetCellCoordinates(box.Max);

		for (int x = min.X; x <= max.X; ++x)
			for (int y = min.Y; y <= max.Y; ++y)
				for (int z = min.Z; z <= max.Z; ++z)
					if (GetCellCollider(query, chunk, x, y, z))
						query.CollisionPairCallback(query.NodeColliderCallback(node->ClientData), query.Collider);
	}

	typedef std::function<void(float x, float y, float z)> PushVertexCallback;
	typedef std::function<void(PushVertexCallback pushTriangle)> PermutationFunction;

	PermutationFunction permutations[16] = {
		[] (PushVertexCallback pushVertex) {},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 0.5f, 0);
			pushVertex(0.5f, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0.5f, 0, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(1, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 0.5f, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(1, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 1, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(0, 0.5f, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 1, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(0.5f, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 1, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(1, 0, 0);
			pushVertex(0.5f, 0, 0);
			pushVertex(0, 0.5f, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 1, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(1, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(1, 1, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(0.5f, 1, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 0.5f, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(0.5f, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0.5f, 0, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, .5f, 0);
			pushVertex(0.5f, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0.5f, 0);
			pushVertex(0, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0.5f, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0.5f, 0);
			pushVertex(0.5f, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0.5f, 0);
			pushVertex(0, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0, 0);
			pushVertex(0.5f, 0, 0);
		},
		[] (PushVertexCallback pushVertex)
		{
			pushVertex(0, 0, 0);
			pushVertex(0, 1, 0);
			pushVertex(1, 1, 0);
			pushVertex(1, 0, 0);
		},
	};

	float lerpEdge(float weight1, float weight2)
	{
		if (weight1 == 0)
			return 1 - weight2;
		else
			return weight1;
	}

	bool TerrainCollider2D::GetCellCollider(QueryData& query, const std::shared_ptr<Chunk>& chunk, int x, int y, int z) const
	{
		FillOccupancy(query.Occupancy, chunk, x, y, z);

		int permutation = GetPermutation(query.Occupancy);

		query.Collider.Clear();
		query.Index = 16 * chunk->GetCoordinates() + Coordinates(x, y, z);

		permutations[permutation]([&query] (float x, float y, float z)
		{
			if (x == 0.5f)
			{
				if (y == 0)
					x = lerpEdge(query.Occupancy[0], query.Occupancy[1]);
				else
					x = lerpEdge(query.Occupancy[2], query.Occupancy[3]);
			}
			else if (y == 0.5f)
			{
				if (x == 0)
					y = lerpEdge(query.Occupancy[0], query.Occupancy[2]);
				else
					y = lerpEdge(query.Occupancy[1], query.Occupancy[3]);
			}

			query.Collider.Push(Vector3(x, y, z, 1) + Vector3(query.Index));
		});

		if (query.Collider.Vertices.size() > 0)
			query.Collider.Finalize();

		return query.Collider.Vertices.size() > 0;
	}

	void TerrainCollider2D::FillOccupancy(float* occupancy, const std::shared_ptr<Chunk>& chunk, int x, int y, int z) const
	{
		const Coordinates& index = chunk->GetCoordinates();

		std::shared_ptr<Chunk> chunks[8] = {
			chunk,
			chunk->FetchChunk(index.X + 1, index.Y,	 index.Z),
			chunk->FetchChunk(index.X,	 index.Y + 1, index.Z),
			chunk->FetchChunk(index.X + 1, index.Y + 1, index.Z),
			chunk->FetchChunk(index.X,	 index.Y,	 index.Z + 1),
			chunk->FetchChunk(index.X + 1, index.Y,	 index.Z + 1),
			chunk->FetchChunk(index.X,	 index.Y + 1, index.Z + 1),
			chunk->FetchChunk(index.X + 1, index.Y + 1, index.Z + 1),
		};

		switch (Axis)
		{
		case Enum::TerrainColliderAxis::XYPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x + 1, y, z);
			occupancy[2] = FetchOccupancy(chunks, x, y + 1, z);
			occupancy[3] = FetchOccupancy(chunks, x + 1, y + 1, z);

			break;
		case Enum::TerrainColliderAxis::XZPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x + 1, y, z);
			occupancy[2] = FetchOccupancy(chunks, x, y, z + 1);
			occupancy[3] = FetchOccupancy(chunks, x + 1, y, z + 1);

			break;
		case Enum::TerrainColliderAxis::ZYPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x, y + 1, z);
			occupancy[2] = FetchOccupancy(chunks, x, y, z + 1);
			occupancy[3] = FetchOccupancy(chunks, x, y + 1, z + 1);

			break;

		case Enum::TerrainColliderAxis::YXPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x, y + 1, z);
			occupancy[2] = FetchOccupancy(chunks, x + 1, y, z);
			occupancy[3] = FetchOccupancy(chunks, x + 1, y + 1, z);

			break;
		case Enum::TerrainColliderAxis::ZXPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x, y, z + 1);
			occupancy[2] = FetchOccupancy(chunks, x + 1, y, z);
			occupancy[3] = FetchOccupancy(chunks, x + 1, y, z + 1);

			break;
		case Enum::TerrainColliderAxis::YZPlane:
			occupancy[0] = FetchOccupancy(chunks, x, y, z);
			occupancy[1] = FetchOccupancy(chunks, x, y, z + 1);
			occupancy[2] = FetchOccupancy(chunks, x, y + 1, z);
			occupancy[3] = FetchOccupancy(chunks, x, y + 1, z + 1);

			break;
		}
	}

	int TerrainCollider2D::GetPermutation(const float* occupancy) const
	{
		int permutation = 0;

		for (int i = 0; i < 4; ++i)
			permutation |= (occupancy[i] > 0) << i;

		return permutation;
	}

	float TerrainCollider2D::FetchOccupancy(const std::shared_ptr<Chunk>* chunks, int x, int y, int z) const
	{
		int chunkX = int(x == 16);
		int chunkY = int(y == 16);
		int chunkZ = int(z == 16);
		int index = (chunkZ << 2) | (chunkY << 1) | chunkX;

		if (chunks[index] != nullptr)
			return chunks[index]->GetCell(Coordinates(x % 16, y % 16, z % 16)).Occupancy;

		return 0;
	}
}