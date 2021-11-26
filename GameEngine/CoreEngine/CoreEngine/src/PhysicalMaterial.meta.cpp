#pragma once

#include "PhysicalMaterial.h"

namespace GraphicsEngine
{
	using Engine::Object;

	Reflect_Inherited(PhysicalMaterial, Object,
		Document_Class("");

		Document("");
		Archivable Class_Member(RGBA, Albedo);

		Document("");
		Archivable Class_Member(float, Roughness);

		Document("");
		Archivable Class_Member(float, Metalness);

		Document("");
		Archivable Class_Member(float, RefractiveIndex);

		Document("");
		Archivable Class_Member(float, Transparency);

		Document("");
		Archivable Class_Member(float, Translucency);

		Document("");
		Archivable Class_Member(float, Emission);
	);
}
