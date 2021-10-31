#pragma once

#include <memory>

#include "Vector3.h"
#include "ObjectReflection.h"

namespace Engine
{
	class Object;
}

namespace GraphicsEngine
{
	class Material;
}

struct SceneRayCastResults
{
	float Distance = -1;
	float Reflectivity = 0;
	Vector3 Intersection;
	Vector3 Normal;
	Vector3 Color;
	Vector3 GlowColor;
	std::shared_ptr<const GraphicsEngine::Material> MaterialProperties;
	std::shared_ptr<Engine::Object> Hit;

	bool operator<(const SceneRayCastResults& other) const
	{
		if (Distance == -1)
			return true;

		return Distance < other.Distance;
	}

	operator std::string() const;

	Base_Class;

	Reflected_Type(SceneRayCastResults);
};

typedef std::function<void(const SceneRayCastResults& results)> CastResultsCallback;

namespace Engine
{
	Define_Value_Type(SceneRayCastResults);
}