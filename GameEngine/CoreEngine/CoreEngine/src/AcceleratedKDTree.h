#pragma once

#include <vector>

#include "Vector3.h"
#include "RGBA.h"

namespace GraphicsEngine
{
	struct KDAxisEnum
	{
		enum KDAxis
		{
			X,
			Y,
			Z,

			None
		};
	};
	
	typedef KDAxisEnum::KDAxis KDAxis;

	class Vector
	{
	public:
		float X = 0;
		float Y = 0;
		float Z = 0;

		Vector(float x = 0, float y = 0, float z = 0) : X(x), Y(y), Z(z) {}
		Vector(const Vector3& other) : X(other.X), Y(other.Y), Z(other.Z) {}
		Vector(const RGBA& other) : X(other.R), Y(other.G), Z(other.B) {}

		operator Vector3() const
		{
			return Vector3(X, Y, Z);
		}

		operator RGBA() const
		{
			return RGBA(X, Y, Z);
		}

		float& operator[](int index) { return (&X)[index]; }
		const float& operator[](int index) const { return (&X)[index]; }
	};

	class Vertex
	{
	public:
		Vector Position;
		Vector Normal;
	};

	class Face
	{
	public:
		int A = -1;
		int B = -1;
		int C = -1;
		float Area = 0;
		Vector Normal;
	};

	class BoundingBox
	{
	public:
		Vector3 Bounds[2] = {};

		Vector3& operator[](int index) { return Bounds[index]; }
		const Vector3& operator[](int index) const { return Bounds[index]; }
	};

	class TreeNode
	{
	public:
		int Index = -1;
		int Left = -1;
		int Right = -1;
		BoundingBox Box;
	};

	struct KDNode
	{
		KDAxis Axis = KDAxis::X;
		float Position = 0;
		int Left = -1;
		int Right = -1;
		int Index = -1;
		int Parent = -1;
		bool IsLeft = false;
	};

	class Mesh
	{
	public:
		BoundingBox Bounds;
		int StartIndex = -1; // index buffer in Faces
		int TriangleCount = 0;
		int TreeHead = -1;
		int KDTreeHead = -1;
	};
	
	struct AcceleratedKDTree
	{
		typedef std::vector<Vertex> VertexVector;
		typedef std::vector<Face> FaceVector;
		typedef std::vector<KDNode> KDNodeVector;
		typedef std::vector<TreeNode> NodeVector;

		struct AccelerationVectors
		{
			const Mesh& mesh;
			KDNodeVector& kdNodes;
			const VertexVector& vertices;
			const FaceVector& faces;
			NodeVector& nodes;
		};

		static int Construct(const Mesh& mesh, KDNodeVector& kdNodes, const VertexVector& vertices, const FaceVector& faces, NodeVector& nodes);

	private:
		static inline const int MaxDepth = 1024;
		static inline const float Epsilon = 0.01f;
		static inline const float MarginThreshold = 0.1f;
		static inline const float MinimumMargin = 0.1f;

		struct Range
		{
			int Start = -1;
			int End = -1;
			
			~Range();
		};

		struct SplitResults
		{
			KDAxis Axis = KDAxis::X;
			float Position = 0;
			int LeftBoxes = 0;
			int RightBoxes = 0;
			int IntersectedBoxes = 0;
			bool Trimmed = false;
			float MarginVolume = 0;

			SplitResults ComputeVolume(const BoundingBox& box);
		};

		struct AxisBounds
		{
			float Min = 0;
			float Max = 0;
		};

		static int Split(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes, int parent, int depth, bool isLeft);
		static void Seperate(AccelerationVectors& accelerator, const SplitResults& results, const Range& boxes, Range& leftNodes, Range& rightNodes);
		static SplitResults FindSplit(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes);
		static int PushNode(AccelerationVectors& accelerator, const KDNode& node);
		static SplitResults FindSplitAxis(AccelerationVectors& accelerator, KDAxis axis, const BoundingBox& bounds, const Range& boxes, bool marginOnly);
		static const SplitResults& CompareSplit(const SplitResults& split1, const SplitResults& split2, const BoundingBox& bounds);
		static float ScoreTrim(const SplitResults& split, const BoundingBox& bounds);
		static float Score(const SplitResults& split);
		static AxisBounds GetBox(AccelerationVectors& accelerator, KDAxis axis, int index);
		static BoundingBox GetBox(AccelerationVectors& accelerator, int index);
		static SplitResults RankPlane(AccelerationVectors& accelerator, KDAxis axis, float position, const BoundingBox& bounds, int totalObjects, int start);
		static SplitResults RankPlaneSlow(AccelerationVectors& accelerator, KDAxis axis, float position, const Range& boxes);
		static int ConstructAabbTree(AccelerationVectors& accelerator, const BoundingBox& bounds, const Range& boxes);
		static BoundingBox GetPairBounds(const BoundingBox& left, const BoundingBox& right);
		static float SurfaceArea(const BoundingBox& box);
		static void RecomputeHeight(int nodeID);
		static void PushAabbNode(AccelerationVectors& accelerator, int& head, int index, int parent, bool fromLeft = false);
	};
}