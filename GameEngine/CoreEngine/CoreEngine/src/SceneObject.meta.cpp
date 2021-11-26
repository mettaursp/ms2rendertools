#include "Scene.h"

#include "Material.h"

namespace GraphicsEngine
{
	using Engine::Object;

	Reflect_Inherited(SceneObject, Object,
		Document_Class("");
		
		//Document("");
		//Archivable Class_Member(std::weak_ptr<Material>, MaterialProperties);

		Document("");
		Archivable Class_Member(std::weak_ptr<PhysicalMaterial>, PhysicalMaterialProperties);

		Document("");
		Archivable Class_Member(bool, Visible);

		Bind_Function(SetMaterial,

			Document("");
			Function_Overload
			(
				Returns_Nothing;
					
				Overload_Parameters
				(
					Document("");
					Function_Parameter(std::shared_ptr<Material>, transformation);
				);

				Bind_Parameters_No_Return(SetMaterial, transformation);
			);
		);

		Bind_Function(GetMaterial,

			Document("");
			Function_Overload
			(
				Document("");
				Overload_Returns(std::shared_ptr<Material>);
					
				Overload_Parameters();

				Bind_Parameters(GetMaterial);
			);
		);

		Document("");
		Register_Lua_Property(MaterialProperties,
			Property_Getter(GetMaterial, std::shared_ptr<Material>);
			
			Property_Setters(
				Bind_Setter(SetMaterial, std::shared_ptr<Material>);
			);
		);
	);
}
