#include "SceneRayCastResults.h"

#include "Object.h"

Reflect_Type(SceneRayCastResults,
	Document_Class("");
		
	Document("");
	Archivable Class_Member(float, Distance);

	Document("");
	Archivable Class_Member(float, Reflectivity);

	Document("");
	Archivable Class_Member(Vector3, Intersection);

	Document("");
	Archivable Class_Member(Vector3, Normal);

	Document("");
	Archivable Class_Member(Vector3, Color);

	Document("");
	Archivable Class_Member(Vector3, GlowColor);

	Document("");
	Archivable Class_Member(std::shared_ptr<Engine::Object>, Hit);
	
	Bind_Function(operator string,

		Document("");
		Function_Overload
		(
			Document("");
			Overload_Returns(std::string);

			Overload_Parameters();

			Bind_Parameters(operator std::string);
		);
	);
);