#pragma once

#include "RGBA.h"
#include "Object.h"

namespace GraphicsEngine
{
	class PhysicalMaterial : public Engine::Object
	{
	public:
		RGBA Albedo = RGBA(1, 1, 1, 0);
		float Roughness = 0.5f;
		float Metalness = 0;
		float RefractiveIndex = 1.1f;
		float Transparency = 0; // (1 - transparency) * diffuse; transparency * transmitted
		float Translucency = 0.01f; // light loss from travel distance
		float Emission = 0;

		void Initialize() {}

		Instantiable;

		Inherits_Class(Object);

		Reflected(PhysicalMaterial);
	};
}