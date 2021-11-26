#include "TerrainColliderAxisEnum.h"

#include "ObjectReflection.h"
#include "ObjectLuaType.h"

namespace Engine
{
	Enum_Definition(TerrainColliderAxis,
		Document_Enum("");
		
		Enum_Item(XYPlane);
		Enum_Item(XZPlane);
		Enum_Item(ZYPlane);

		Enum_Item(YXPlane);
		Enum_Item(ZXPlane);
		Enum_Item(YZPlane);
	);
}