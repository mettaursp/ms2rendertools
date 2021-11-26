#pragma once

#include "EnumDeclaration.h"

namespace EnumTypeStorage
{
	struct TerrainColliderAxisEnum
	{
		enum TerrainColliderAxis
		{
			XYPlane,
			XZPlane,
			ZYPlane,

			YXPlane,
			ZXPlane,
			YZPlane
		};
	};
}

namespace Enum
{
	typedef EnumTypeStorage::TerrainColliderAxisEnum::TerrainColliderAxis TerrainColliderAxis;
}

namespace Engine
{
	Declare_Enum(TerrainColliderAxis);
}