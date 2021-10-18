#include "AcceleratedKDTree.h"

#include <iostream>
#include <algorithm>

#include "AabbTree.h"

namespace GraphicsEngine
{
	struct BoxData
	{
		float Min = 0;
		float Max = 0;
		int Count = 1;
		int TotalBefore = 0;
		float LargestBefore = 0;
	};

	typedef std::vector<int> IntVector;
	typedef std::vector<float> FloatVector;
	typedef std::vector<BoxData> BoxVector;

	IntVector faceReferences;
	FloatVector splittingPlanes;
	BoxVector objects;


	AcceleratedKDTree::Range::~Range()
	{
		if (int(faceReferences.size()) != End)
			std::cout << "buffer pop failed" << std::endl;

		if (int(faceReferences.size()) != Start)
			faceReferences.resize(Start);
	}

	int AcceleratedKDTree::Construct(const Mesh& mesh, KDNodeVector& kdNodes, const VertexVector& vertices, const FaceVector& faces, NodeVector& nodes)
	{
		faceReferences.clear();
		faceReferences.reserve(mesh.TriangleCount);

		for (int i = 0; i < mesh.TriangleCount; ++i)
			faceReferences.push_back(mesh.StartIndex + i);

		AccelerationVectors accelerator{ mesh, kdNodes, vertices, faces, nodes };

		return Split(accelerator, mesh.Bounds, Range{ 0, int(faceReferences.size()) }, -1, 0, false);
	}

	int AcceleratedKDTree::Split(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes, int parent, int depth, bool isLeft)
	{
		SplitResults results = FindSplit(accelerator, bounds, boxes);

		if (depth > MaxDepth || (!results.Trimmed && (results.LeftBoxes == 0 || results.RightBoxes == 0)))
		{
			if (depth > MaxDepth)
				std::cout << "hit max depth" << std::endl;

			return PushNode(accelerator, KDNode{ KDAxis::None, 0, -1, -1, ConstructAabbTree(accelerator, bounds, boxes), parent });
		}

		BoundingBox childBounds[2] = { bounds, bounds };

		childBounds[0][1][results.Axis] = results.Position;
		childBounds[1][0][results.Axis] = results.Position;

		int nodeID = PushNode(accelerator, KDNode{ results.Axis, results.Position, -1, -1, -1, parent, isLeft });
		KDNode& node = accelerator.kdNodes[nodeID];

		if (results.Trimmed)
		{
			bool splitLeft = results.RightBoxes == 0;

			int children[2] = {
				PushNode(accelerator, KDNode{ KDAxis::None, 0, -1, -1, -1, nodeID, !splitLeft }),
				Split(accelerator, childBounds[splitLeft], boxes, nodeID, depth + 1, splitLeft)
			};

			accelerator.kdNodes[nodeID].Left = children[splitLeft];
			accelerator.kdNodes[nodeID].Right = children[!splitLeft];
		}
		else
		{
			int stackIndex = int(faceReferences.size());

			Range leftNodes{ stackIndex, stackIndex };
			Range rightNodes{ stackIndex, stackIndex };

			Seperate(accelerator, results, boxes, leftNodes, rightNodes);

			accelerator.kdNodes[nodeID].Left = Split(accelerator, childBounds[0], leftNodes, nodeID, depth + 1, true);
			accelerator.kdNodes[nodeID].Right = Split(accelerator, childBounds[1], rightNodes, nodeID, depth + 1, false);
		}

		return nodeID;
	}

	void AcceleratedKDTree::Seperate(AccelerationVectors& accelerator, const SplitResults& results, const Range& boxes, Range& leftNodes, Range& rightNodes)
	{
		SplitResults res = RankPlaneSlow(accelerator, results.Axis, results.Position, boxes);

		for (int i = boxes.Start; i < boxes.End; ++i)
			if (!(GetBox(accelerator, results.Axis, i).Min >= results.Position - Epsilon))
				faceReferences.push_back(faceReferences[i]);

		leftNodes.End = int(faceReferences.size());
		rightNodes.Start = int(faceReferences.size());

		for (int i = boxes.Start; i < boxes.End; ++i)
			if (!(GetBox(accelerator, results.Axis, i).Max <= results.Position + Epsilon))
				faceReferences.push_back(faceReferences[i]);

		rightNodes.End = int(faceReferences.size());

		if (leftNodes.End - leftNodes.Start != res.LeftBoxes + res.IntersectedBoxes)
			std::cout << "mismatch in left; expected " << (leftNodes.End - leftNodes.Start) << ", got " << (res.LeftBoxes + res.IntersectedBoxes) << std::endl;

		if (rightNodes.End - rightNodes.Start != res.RightBoxes + res.IntersectedBoxes)
			std::cout << "mismatch in right; expected " << (rightNodes.End - rightNodes.Start) << ", got " << (res.RightBoxes + res.IntersectedBoxes) << std::endl;
	}

	AcceleratedKDTree::SplitResults AcceleratedKDTree::FindSplit(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes)
	{
		SplitResults splitX = FindSplitAxis(accelerator, KDAxis::X, bounds, boxes, false);
		SplitResults splitY = FindSplitAxis(accelerator, KDAxis::Y, bounds, boxes, splitX.Trimmed);
		SplitResults splitZ = FindSplitAxis(accelerator, KDAxis::Z, bounds, boxes, splitX.Trimmed || splitY.Trimmed);

		if (splitX.Trimmed || splitY.Trimmed || splitZ.Trimmed)
		{
			if (splitX.MarginVolume > splitY.MarginVolume && splitX.MarginVolume > splitZ.MarginVolume)
				return splitX;
			else if (splitY.MarginVolume > splitZ.MarginVolume)
				return splitY;
			else
				return splitZ;
		}

		return CompareSplit(CompareSplit(splitX, splitY, bounds), splitZ, bounds);
	}

	int AcceleratedKDTree::PushNode(AccelerationVectors& accelerator, const KDNode& node)
	{
		int nodeID = int(accelerator.kdNodes.size());

		accelerator.kdNodes.push_back(node);

		return nodeID;
	}

	const AcceleratedKDTree::SplitResults& AcceleratedKDTree::CompareSplit(const SplitResults& split1, const SplitResults& split2, const BoundingBox& bounds)
	{
		bool trim1 = split1.LeftBoxes == 0 || split1.RightBoxes == 0;
		bool trim2 = split2.LeftBoxes == 0 || split2.RightBoxes == 0;

		float score1 = 0;
		float score2 = 0;

		if (trim1 && trim2)
		{
			score1 = ScoreTrim(split1, bounds);
			score2 = ScoreTrim(split2, bounds);
		}
		else
		{
			score1 = Score(split1);
			score2 = Score(split2);
		}

		return score1 < score2 ? split1 : split2;
	}

	float AcceleratedKDTree::ScoreTrim(const SplitResults& split, const BoundingBox& bounds)
	{
		float length = split.LeftBoxes == 0 ? split.Position - bounds[0][split.Axis] : bounds[1][split.Axis] - split.Position;

		Vector size{
			bounds[1].X - bounds[0].X,
			bounds[1].Y - bounds[0].Y,
			bounds[1].Z - bounds[0].Z
		};

		size[split.Axis] = length;

		int separated = std::max(split.LeftBoxes, split.RightBoxes) + split.IntersectedBoxes;

		return float(separated * separated + split.IntersectedBoxes * split.IntersectedBoxes) / (size.X * size.Y * size.Z);
	}

	float AcceleratedKDTree::Score(const SplitResults& split)
	{
		int leftScore = split.LeftBoxes + split.IntersectedBoxes;
		int rightScore = split.RightBoxes + split.IntersectedBoxes;

		return float(leftScore * leftScore + rightScore * rightScore);
	}

	bool floatEquals(float a, float b, float epsilon)
	{
		return std::abs(a - b) < epsilon;
	}

	void CullDuplicateObjects(float epsilon);
	void CullDuplicatePlanes(float epsilon);

	AcceleratedKDTree::SplitResults AcceleratedKDTree::FindSplitAxis(AccelerationVectors& accelerator, KDAxis axis, const BoundingBox& bounds, const Range& boxes, bool marginOnly)
	{
		objects.clear();

		float boxMin = bounds[1][axis];
		float boxMax = bounds[0][axis];

		for (int i = boxes.Start; i < boxes.End; ++i)
		{
			AxisBounds box = GetBox(accelerator, axis, i);

			boxMin = std::min(boxMin, box.Min);
			boxMax = std::max(boxMax, box.Max);

			objects.push_back(BoxData{ box.Min, box.Max });
		}

		boxMin -= 2 * Epsilon;
		boxMax += 2 * Epsilon;

		float extents = bounds[1][axis] - bounds[0][axis];
		float marginBefore = std::max(boxMin - bounds[0][axis], 0.f);
		float marginAfter = std::max(bounds[1][axis] - boxMax, 0.f);

		if (std::max(marginBefore, marginAfter) > std::max(MarginThreshold * extents, MinimumMargin))
		{
			if (marginBefore > marginAfter)
				return SplitResults{ axis, std::max(boxMin, bounds[0][axis]), 0, boxes.End - boxes.Start, 0, true }.ComputeVolume(bounds);
			else
				return SplitResults{ axis, std::min(boxMax, bounds[1][axis]), boxes.End - boxes.Start, 0, 0, true }.ComputeVolume(bounds);
		}
		else if (marginOnly)
			return SplitResults{ axis, 0, 0, 0, 0, false };

		std::sort(objects.begin(), objects.end(), [](const BoxData& left, const BoxData& right) -> bool
			{
				if (floatEquals(left.Min, right.Min, Epsilon))
					return left.Max < right.Max;

				return left.Min < right.Min;
			});

		CullDuplicateObjects(Epsilon);

		int totalObjects = boxes.End - boxes.Start;

		SplitResults best = RankPlane(accelerator, axis, 0.5f * (bounds[0][axis] + bounds[1][axis]), bounds, totalObjects, int(objects.size()) - 1);

		splittingPlanes.clear();

		for (int i = 0; i < int(objects.size()); ++i)
		{
			if (objects[i].Min > bounds[0][axis])
				splittingPlanes.push_back(objects[i].Min);

			if (objects[i].Min < bounds[1][axis])
				splittingPlanes.push_back(objects[i].Max);
		}

		std::sort(splittingPlanes.begin(), splittingPlanes.end());

		CullDuplicatePlanes(Epsilon);

		int rangeBounds = int(objects.size()) - 1;

		for (int i = int(splittingPlanes.size()) - 1; i >= 0; --i)
			best = CompareSplit(best, RankPlane(accelerator, axis, splittingPlanes[i], bounds, totalObjects, rangeBounds), bounds);

		return best;
	}

	AcceleratedKDTree::SplitResults AcceleratedKDTree::SplitResults::ComputeVolume(const BoundingBox& box)
	{
		float marginSize = 0;

		if (LeftBoxes == 0)
			marginSize = Position - box[0][Axis];
		else
			marginSize = box[1][Axis] - Position;

		float size[3] = {
			box[1][0] - box[0][0],
			box[1][1] - box[0][1],
			box[1][2] - box[0][2]
		};

		size[Axis] = marginSize;

		MarginVolume = size[0] * size[1] * size[2];

		return *this;
	}

	void CullDuplicateObjects(float epsilon)
	{
		int index = 0;
		int total = 0;
		float maxSpan = 0;

		for (int i = 1; i < int(objects.size()); ++i)
		{
			if (!(floatEquals(objects[i].Min, objects[index].Min, epsilon) && floatEquals(objects[i].Max, objects[index].Max, epsilon)))
			{
				++index;

				maxSpan = std::max(maxSpan - (objects[i].Min - objects[index].Min), objects[i].Max - objects[i].Min);

				objects[index] = objects[i];
				objects[index].LargestBefore = maxSpan;
				objects[index].TotalBefore = total;
			}
			else
				++objects[index].Count;

			++total;
		}

		objects.resize(std::min(index + 1, int(objects.size())));
	}

	void CullDuplicatePlanes(float epsilon)
	{
		int index = 0;

		for (int i = 1; i < int(splittingPlanes.size()); ++i)
		{
			if (!(floatEquals(splittingPlanes[i], splittingPlanes[index], epsilon)))
			{
				++index;

				splittingPlanes[index] = splittingPlanes[i];
			}
		}

		splittingPlanes.resize(std::min(index + 1, int(splittingPlanes.size())));
	}

	AcceleratedKDTree::AxisBounds AcceleratedKDTree::GetBox(AccelerationVectors& accelerator, KDAxis axis, int index)
	{
		const Face& face = accelerator.faces[faceReferences[index]];

		float a = accelerator.vertices[face.A].Position[axis];
		float b = accelerator.vertices[face.B].Position[axis];
		float c = accelerator.vertices[face.C].Position[axis];

		return AxisBounds{
			std::min(std::min(a, b), c),
			std::max(std::max(a, b), c)
		};
	}

	BoundingBox AcceleratedKDTree::GetBox(AccelerationVectors& accelerator, int index)
	{
		AxisBounds x = GetBox(accelerator, KDAxis::X, index);
		AxisBounds y = GetBox(accelerator, KDAxis::Y, index);
		AxisBounds z = GetBox(accelerator, KDAxis::Z, index);

		return BoundingBox{ Vector3(x.Min, y.Min, z.Min), Vector3(x.Max, y.Max, z.Max) };
	}

	AcceleratedKDTree::SplitResults AcceleratedKDTree::RankPlane(AccelerationVectors& accelerator, KDAxis axis, float position, const BoundingBox& bounds, int totalObjects, int start)
	{
		if (position <= bounds[0][axis])
			return SplitResults{ axis, position, 0, totalObjects, 0, false, 0 };
		else if (position >= bounds[1][axis])
			return SplitResults{ axis, position, totalObjects, 0, 0, false, 0 };

		int before = 0;
		int after = 0;
		int intersecting = 0;

		int rangeStart = 0;
		int rangeEnd = start;

		while (rangeEnd > rangeStart + 1)
		{
			int mid = rangeStart + (rangeEnd - rangeStart) / 2;

			if (objects[mid].Min >= position - Epsilon)
				rangeEnd = mid;
			else
				rangeStart = mid;
		}

		for (int i = rangeEnd; i >= 0; --i)
		{
			if (objects[i].Min >= position - Epsilon)
				before += objects[i].Count;
			else if (objects[i].Max <= position + Epsilon)
				after += objects[i].Count;
			else
				intersecting += objects[i].Count;
		}

		int adding = totalObjects - before - after - intersecting;

		before += adding;

		return SplitResults{ axis, position, before, after, intersecting, false, 0 };
	}

	AcceleratedKDTree::SplitResults AcceleratedKDTree::RankPlaneSlow(AccelerationVectors& accelerator, KDAxis axis, float position, const Range& boxes)
	{
		int before = 0;
		int after = 0;
		int intersecting = 0;

		for (int i = boxes.Start; i < boxes.End; ++i)
		{
			if (GetBox(accelerator, axis, i).Max <= position + Epsilon)
				++before;
			else if (GetBox(accelerator, axis, i).Min >= position - Epsilon)
				++after;
			else
				++intersecting;
		}

		return SplitResults{ axis, position, before, after, intersecting, false, 0 };
	}

	struct BoxReference
	{
		int ID = -1;
		int Index = -1;
		int Left = -1;
		int Right = -1;
		int Parent = -1;
		int Height = -1;
		AcceleratedKDTree::AccelerationVectors* accelerator = nullptr;

		BoxReference(AcceleratedKDTree::AccelerationVectors& accelerator, int i, int index, const BoundingBox& box) : accelerator(&accelerator), Index(i)
		{
			ID = int(accelerator.nodes.size());

			accelerator.nodes.push_back(TreeNode{ faceReferences[index], -1, -1, box });
		}

		BoxReference() {}

		TreeNode& Get()
		{
			return accelerator->nodes[ID];
		}

		TreeNode* operator*()
		{
			return &Get();
		}

		TreeNode* operator->()
		{
			return &Get();
		}
	};

	typedef std::vector<BoxReference> BoxReferenceVector;

	BoxReferenceVector boxNodes;

	BoxReference NewBox(AcceleratedKDTree::AccelerationVectors& accelerator, int index, const BoundingBox& box)
	{
		boxNodes.push_back(BoxReference(accelerator, int(boxNodes.size()), index, box));

		return boxNodes.back();
	}

	AabbTree NewNodes;

	int AcceleratedKDTree::ConstructAabbTree(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes)
	{
		return -1;
		//if (boxes.End - boxes.Start == 0)
		//	return -1;
		//
		//NewNodes.Clear();
		//
		//for (int i = boxes.Start + 1; i < boxes.End; ++i)
		//{
		//	GetBox(accelerator, i);
		//
		//
		//	BoxReference node = NewBox(accelerator, i, );
		//
		//	PushAabbNode(accelerator, head, node.Index, head);
		//}
		//
		//return -1;
		//
		//boxNodes.clear();
		//
		//BoxReference node = NewBox(accelerator, boxes.Start, GetBox(accelerator, boxes.Start));
		//
		//int head = node.ID;
		//
		//for (int i = boxes.Start + 1; i < boxes.End; ++i)
		//{
		//	BoxReference node = NewBox(accelerator, i, GetBox(accelerator, i));
		//
		//	PushAabbNode(accelerator, head, node.Index, head);
		//}
		//
		//return head;
	}

	BoundingBox AcceleratedKDTree::GetPairBounds(const BoundingBox& left, const BoundingBox& right)
	{
		return BoundingBox{
			Vector3(std::min(left[0].X, right[0].X), std::min(left[0].Y, right[0].Y), std::min(left[0].Z, right[0].Z)),
			Vector3(std::min(left[1].X, right[1].X), std::min(left[1].Y, right[1].Y), std::min(left[1].Z, right[1].Z)),
		};
	}

	float AcceleratedKDTree::SurfaceArea(const BoundingBox& box)
	{
		Vector3 diff = box[1] - box[0];

		return 2 * diff.X * diff.Y + 2 * diff.Y * diff.Z + 2 * diff.X * diff.Z;
	}

	void AcceleratedKDTree::RecomputeHeight(int nodeID)
	{
		if (nodeID == -1)
			return;

		BoxReference* node = &boxNodes[nodeID];
		BoxReference* leftNode = &boxNodes[node->Left];
		BoxReference* rightNode = &boxNodes[node->Right];

		int leftHeight = leftNode->Height;
		int rightHeight = rightNode->Height;

		

		if (node == nullptr)
			return;
		/*
		int leftHeight = node->GetLeft()->Height;
		int rightHeight = node->GetRight()->Height;

		node->Box = GetPairBounds(node->GetLeft()->Box, node->GetRight()->Box);

		if (leftHeight - rightHeight > 1 || rightHeight - leftHeight > 1)
		{
			int* connection = &Root;

			if (node->Parent != -1)
			{
				if (node->GetParent()->Left == node->ID)
					connection = &node->GetParent()->Left;
				else
					connection = &node->GetParent()->Right;
			}

			Rotate(node, connection);
		}
		else
		{
			if (leftHeight > rightHeight)
				node->Height = leftHeight + 1;
			else
				node->Height = rightHeight + 1;
		}

		if (node->Parent != -1)
			RecomputeHeight(node->GetParent());*/
	}

	void AcceleratedKDTree::PushAabbNode(AccelerationVectors& accelerator, int& head, int index, int parent, bool fromLeft)
	{
		return;

		int* connection = &head;

		BoxReference* node = &boxNodes[index];
		BoxReference* parentNode = &boxNodes[parent];

		if (parentNode->Parent != -1)
		{
			if (fromLeft)
				connection = &boxNodes[parentNode->Parent]->Left;
			else
				connection = &boxNodes[parentNode->Parent]->Right;
		}

		int nodeID = node->ID;
		int parentID = parentNode->ID;

		if (parentNode->Index == -1)
		{
			BoxReference bridgeData = NewBox(accelerator, -1, GetPairBounds(parentNode->Get().Box, node->Get().Box));
			BoxReference* bridge = &boxNodes[bridgeData.ID];

			node = &boxNodes[index];
			parentNode = &boxNodes[parent];

			if (parentNode->Parent != -1)
			{
				if (fromLeft)
					connection = &boxNodes[parentNode->Parent]->Left;
				else
					connection = &boxNodes[parentNode->Parent]->Right;
			}

			bridge->Left = parentNode->ID;
			bridge->Right = node->ID;
			bridge->Height = 1;
			bridge->Parent = parentNode->Parent;

			parentNode->Parent = bridge->ID;
			node->Parent = bridge->ID;

			*connection = bridge->ID;

			if (bridge->Parent != -1)
				RecomputeHeight(bridge->Parent);

			return;
		}

		BoxReference* left = &boxNodes[parentNode->Left];
		BoxReference* right = &boxNodes[parentNode->Right];

		BoundingBox newBoundsLeft = GetPairBounds(left->Get().Box, node->Get().Box);
		BoundingBox newBoundsRight = GetPairBounds(right->Get().Box, node->Get().Box);

		float surfaceAreaLeft = SurfaceArea(left->Get().Box);
		float surfaceAreaRight = SurfaceArea(right->Get().Box);

		float newSurfaceAreaLeft = SurfaceArea(newBoundsLeft);
		float newSurfaceAreaRight = SurfaceArea(newBoundsRight);

		float changeLeft = newSurfaceAreaLeft - surfaceAreaLeft;
		float changeRight = newSurfaceAreaRight - surfaceAreaRight;

		if (changeRight <= changeLeft)
			PushAabbNode(accelerator, head, index, right->ID, false);
		else
			PushAabbNode(accelerator, head, index, left->ID, true);
	}
}