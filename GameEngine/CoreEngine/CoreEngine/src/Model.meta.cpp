#include "Model.h"

namespace GraphicsEngine
{
	using Engine::Object;

	Reflect_Inherited(Model, SceneObject,
		Document_Class("");
		
		Document("");
		Archivable Class_Member(std::weak_ptr<Engine::ModelAsset>, Asset);

		Document("");
		Archivable Class_Member(float, Reflectivity);

		Document("");
		Archivable Class_Member(RGBA, Color);

		Document("");
		Archivable Class_Member(RGBA, GlowColor);

		Bind_Function(GetBoundingBox,
		
			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(Aabb);
			
				Overload_Parameters();
			
				Bind_Parameters(GetBoundingBox);
			);
		);
	);
}
