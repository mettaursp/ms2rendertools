#pragma once

#include <vector>

extern "C" {
#include <glew.h>
}
#include "Vector3.h"
#include "RGBA.h"
#include "VertexModeEnum.h"
#include "Ray.h"
#include "SceneRayCastResults.h"
#include "AabbTree.h"

class ObjParser;
class PlyParser;

struct Vector2Raw
{
	float X = 0;
	float Y = 0;

	Vector2Raw(float x = 0, float y = 0) : X(x), Y(y) {}
	Vector2Raw(const Vector3& vector) : X(vector.X), Y(vector.Y) {}

	operator Vector3()
	{
		return Vector3(X, Y);
	}
};

struct Vector3Raw
{
	float X = 0;
	float Y = 0;
	float Z = 0;

	Vector3Raw(float x = 0, float y = 0, float z = 0) : X(x), Y(y), Z(z) {}
	Vector3Raw(const Vector3& vector) : X(vector.X), Y(vector.Y), Z(vector.Z) {}

	operator Vector3() const
	{
		return Vector3(X, Y, Z);
	}

	Vector3Raw operator+(const Vector3Raw& other) const
	{
		return Vector3Raw(X + other.X, Y + other.Y, Z + other.Z);
	}

	Vector3Raw operator-(const Vector3Raw& other) const
	{
		return Vector3Raw(X - other.X, Y - other.Y, Z - other.Z);
	}

	Vector3Raw operator*(float scalar) const
	{
		return Vector3Raw(scalar * X, scalar * Y, scalar * Z);
	}
};

Vector3Raw operator*(float scalar, const Vector3Raw& vector);

struct VertexData
{
	Vector3Raw Position, Normal;
	Vector2Raw UV;
	Vector3 TVector, BVector;
};

struct TriangleData
{
	int VertexA = -1;
	int VertexB = -1;
	int VertexC = -1;
	int Index = -1;
};

class MeshData
{
public:
	typedef std::vector<VertexData> VertexVector;
	typedef std::vector<int> IntVector;
	typedef std::vector<TriangleData> TriangleVector;
	typedef std::function<void(const SceneRayCastResults& results)> CastResultsCallback;

	Enum::VertexMode Mode;

	VertexVector VertexBuffer;
	IntVector IndexBuffer;

	GLenum DrawMode = GL_TRIANGLES;

	MeshData(Enum::VertexMode mode = Enum::VertexMode::Seperate);
	MeshData(const ObjParser* parser, Enum::VertexMode mode = Enum::VertexMode::Seperate);
	MeshData(const PlyParser* parser, Enum::VertexMode mode = Enum::VertexMode::Seperate);
	MeshData(const MeshData* meshData, Enum::VertexMode mode = Enum::VertexMode::Seperate);
	MeshData(const MeshData& other);

	void UpdateBounds();
	void Initialize(const ObjParser* parser);
	void Initialize(const PlyParser* parser);
	void InitializeWireframe(const ObjParser* parser);
	void InitializeWireframe(const PlyParser* parser);
	void GenerateVertexNormals(const MeshData* meshData);
	void GenerateFaceNormals(const MeshData* meshData);
	void GenerateCylinderMap(const MeshData* meshData);
	void GenerateSphereMap(const MeshData* meshData);
	void GenerateCubeMap(const MeshData* meshData);
	void GenerateTBNDebug(const MeshData* meshData);
	Vector3 GetMinimumCorner() const;
	Vector3 GetMaximumCorner() const;
	Vector3 GetCenter() const;
	Vector3 GetSize() const;
	void PushTriangle(int vertexA, int vertexB, int vertexC);
	void ConfigurePartitioning();
	void CastRay(const Ray& ray, const CastResultsCallback& callback) const;
	const AabbTree& GetTree() const { return TrianglePartition; }
	int GetTriangle(const AabbTree::Node* node) const { return node->GetData<TriangleData>()->Index; }

	MeshData& operator=(const MeshData& other);

private:
	void Smoothen();

	Vector3 Minimum;
	Vector3 Maximum;

	TriangleVector Triangles;
	AabbTree TrianglePartition;
};

