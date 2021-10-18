#pragma once

#include "RGBA.h"
#include "Object.h"

namespace GraphicsEngine
{
	class Material : public Engine::Object
	{
	public:
		RGBA Diffuse = RGBA(0.5f, 0.5f, 0.5f, 1);
		RGBA Specular = RGBA(0.5f, 0.5f, 0.5f, 1);
		RGBA Ambient = RGBA(0.1f, 0.1f, 0.1f, 1);
		RGBA Emission = RGBA(0, 0, 0, 0);
		int Shininess = 1;

		void Initialize() {}

		Instantiable;

		Inherits_Class(Object);

		Reflected(Material);
	};
}
